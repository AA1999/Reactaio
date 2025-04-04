//
// Created by arshia on 2/25/23.
//

#include "warn_wrapper.h"
#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../mod_action.h"


#include <algorithm>
#include <dpp/dpp.h>

void warn_wrapper::wrapper_function() {

	check_permissions();

	if(cancel_operation)
		return;

	process_warnings();
	process_response();
}

void warn_wrapper::check_permissions() {
	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();
	auto const author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	bool ignore_owner_repeat{false};

	pqxx::work transaction{*command.connection};
	auto protected_roles_query = transaction.exec_prepared("protected_roles", std::to_string(command.guild->id));
	transaction.commit();

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check ifthe bot has Moderate Members permission.");
	}

	auto const protected_roles = get_protected_roles();

	for(auto const& member: members) {
		auto member_roles = get_roles_sorted(*member);
		auto member_top_role = *member_roles.begin();

		if(command.author->user_id == member->user_id) { // ifforsome reason you decided to warn yourself lol
			if(member->is_guild_owner()) {
				errors.emplace_back("❌ Why are you warning yourself, server owner? lmfao");
				ignore_owner_repeat = true;
			}
			else {
				errors.emplace_back("❌ You can't warn yourself lmao.");
			}
			cancel_operation = true;
		}

		if(!ignore_owner_repeat && member->is_guild_owner()) {
			errors.emplace_back("❌ You can't warn the server owner lmfao.");
			cancel_operation = true;
		}

		if(member->get_user()->is_bot()) {
			errors.emplace_back("❌ Cannot warn a bot. What would that even do lol?");
			cancel_operation = true;
		}

		if(command.bot->me.id == member->user_id) { // ifyou decided to warn the bot (ReactAIO)
			errors.emplace_back("❌ Can't warn myself lmfao.");
			cancel_operation = true;
		}

		if(!protected_roles.empty()) {

			shared_vector<dpp::role> member_protected_roles;

			reactaio::set_intersection(protected_roles, member_roles, member_protected_roles);

			if(!member_protected_roles.empty()) { // ifmember has any of the protected roles.
				cancel_operation = true;
				std::vector<std::string> role_mentions;
				std::ranges::transform(member_protected_roles, std::back_inserter(role_mentions), [](const role_ptr& role){
					return role->get_mention();
				});
				std::string role_mentions_str = join(role_mentions, " , ");
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot warn.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to warn. Please "
										 "move the bot role above the members and below your staff roles.",
										 member->get_mention()));
			cancel_operation = true;
		}

		if(member_top_role->position > author_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than you do. You can't warn them.",
										 member->get_mention()));
			cancel_operation = true;
		}
	}

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while warning member(s): ")
								  .set_color(ERROR_COLOR)
								  .set_timestamp(time_now);
		for(auto const &error: organized_errors) {
			auto embed{base_embed};
			embed.set_description(error);
			error_message.add_embed(embed);
		}
		if(command.interaction) {
			error_message.set_flags(dpp::m_ephemeral);
			if(organized_errors.size() == 1)
				(*command.interaction)->edit_response(error_message);
			else {
				message_paginator paginator{error_message, command};
				paginator.start();
			}
		}
		else
			invoke_error_webhook();
	}
}

void warn_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, member_ptr const &member) {
	if(completion.is_error()) {
		auto error = completion.get_error();
		errors.push_back(std::format("Unable to DM user **{}**. Warning registered. Error{}: {}",
									 member->get_user()->format_username(), error.code, error.human_readable));
		members_with_errors.insert(member);
	}
	log_modcase(internal::mod_action_name::WARN);
}

void warn_wrapper::process_warnings() {
	for(auto const& member : members) {
		if(command.interaction) { // ifthis is automod, DMing lots of users WILL result in a ratelimit
			auto const warning_message = std::format("You have been warned in {} by {}. Reason: {}.", command.guild->name,
											   member->get_user()->format_username(), command.reason);
			command.bot->direct_message_create(member->user_id, dpp::message(warning_message), [this, member](auto const& completion){
				lambda_callback(completion, member);
			});
		}
	}
}

void warn_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author->get_user();

	if(has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
				.set_title("Error while warning member(s): ")
				.set_color(ERROR_COLOR)
				.set_timestamp(time_now);
		if(command.interaction) {
			if(format_split.size() == 1) {
				base_embed.set_description(format_split[0]);
				message.add_embed(base_embed);
			}
			else {
				for(auto const &error: format_split) {
					auto embed{base_embed};
					embed.set_description(error);
					message.add_embed(embed);
				}
			}
			if(are_all_errors())
				message.set_flags(dpp::m_ephemeral); // if there's no successful warn, no reason to show the errors publicly.
		}
		else
			invoke_error_webhook();
	}
	if(!are_all_errors()) {
		shared_vector<dpp::guild_member> warned_members;
		std::vector<std::string> warned_usernames;
		std::vector<std::string> warned_mentions;

		reactaio::set_difference(members, members_with_errors, warned_members);

		std::ranges::transform(warned_members, std::back_inserter(warned_usernames), [](member_ptr const& member) {
			return std::format("**{}**", member->get_user()->format_username());
		});

		std::ranges::transform(warned_members, std::back_inserter(warned_mentions), [](member_ptr const& member) {
			return member->get_mention();
		});

		auto usernames = join(warned_usernames, ", ");
		auto mentions  = join(warned_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if(warned_members.size() == 1) {
			title		= "Warned";
			description = std::format("{} has been warned.", usernames);
			gif_url		= "https://tenor.com/view/will-smith-chris-rock-jada-pinkett-smith-oscars2022-smack-gif-25234614";
		}
		else {
			title		= "Mass warned";
			description = std::format("{} have been warned.", usernames);
			gif_url		= "https://gfycat.com/agitatedincomparableicelandicsheepdog";
		}

		auto time_now	= std::time(nullptr);
		auto reason_str = std::string{command.reason};

		auto response = dpp::embed()
				.set_color(RESPONSE_COLOR)
				.set_title(title)
				.set_description(description)
				.set_image(gif_url)
				.set_timestamp(time_now);
		response.add_field("Moderator: ", author_user->get_mention());
		response.add_field("Reason: ", reason_str);

		message.add_embed(response);

		// Modlogs

		auto transaction = pqxx::work{*command.connection};
		auto query = transaction.exec_prepared1("modlog", std::to_string(command.guild->id));
		transaction.commit();

		if(!query["modlog"].is_null()) {
			auto modlog_webhook = dpp::webhook{query["modlog"].as<std::string>()};
			std::string embed_title, embed_image_url;
			if(warned_members.size() > 1)
				embed_title = "Members warned: ";
			else {
				embed_title = "Member warned: ";
				embed_image_url = warned_members.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto warn_log = dpp::embed()
					.set_color(LOG_COLOR)
					.set_title(embed_title)
					.set_thumbnail(embed_image_url)
					.set_timestamp(time_now)
					.set_description(std::format("{} have been warned.", usernames))
					.add_field("Moderator: ", command.author->get_mention())
					.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(warn_log);
			command.bot->execute_webhook(modlog_webhook, log);
		}
		if(!query["public_modlog"].is_null()) {
			auto public_modlog_webhook = dpp::webhook{query["public_modlog"].as<std::string>()};
			std::string embed_title, embed_image_url;
			if(warned_members.size() > 1)
				embed_title = "Members warned: ";
			else {
				embed_title = "Member warned: ";
				embed_image_url = warned_members.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto warn_log = dpp::embed()
					.set_color(LOG_COLOR)
					.set_title(embed_title)
					.set_thumbnail(embed_image_url)
					.set_timestamp(time_now)
					.set_description(std::format("{} have been warned.", usernames))
					.add_field("Moderator: ", command.author->get_mention())
					.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(warn_log);
			command.bot->execute_webhook(public_modlog_webhook, log);
		}
	}
	if(command.interaction) {
		if(message.embeds.size() == 1)
			(*command.interaction)->edit_response(message);
		else {
			message_paginator paginator{message, command};
			paginator.start();
		}
		log_command_invoke(internal::mod_action_name::WARN);
	}
	else
		command.bot->message_create(message);

}


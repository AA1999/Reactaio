//
// Created by arshia on 2/15/23.
//

#include "kick_wrapper.h"

#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../mod_action.h"

#include <algorithm>
#include <dpp/dpp.h>
#include <format>
#include <iterator>
#include <string>
#include <vector>

void kick_wrapper::wrapper_function() {
	check_permissions();

	if(cancel_operation)
		return;

	process_kicks();
	process_response();
}

void kick_wrapper::check_permissions() {
	auto const bot_member = dpp::find_guild_member(command.guild->id, command.bot->me.id);

	auto bot_roles = get_roles_sorted(bot_member);
	auto bot_top_role = bot_roles.front();

	auto author_roles = get_roles_sorted(*command.author);
	auto author_top_role = author_roles.front();

	bool ignore_owner_repeat{false};

	pqxx::work transaction{*command.connection};
	auto protected_roles_query = transaction.exec_prepared("protected_roles", std::to_string(command.guild->id));
	transaction.commit();

	if(!bot_top_role->has_kick_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot has Kick Members permission.");
	}

	shared_vector<dpp::role> protected_roles;

	if(!protected_roles_query.empty()) {
		auto protected_roles_field = protected_roles_query[0]["protected_roles"];
		internal::unique_vector<dpp::snowflake> protected_role_snowflakes = parse_psql_array<dpp::snowflake>(protected_roles_field);
		reactaio::transform(protected_role_snowflakes, protected_roles, [](auto const& role_id) {
			return std::make_shared<dpp::role>(*dpp::find_role(role_id));
		});
	}


	for(auto const& member: members) {
		auto member_roles = get_roles_sorted(*member);
		auto member_top_role = *member_roles.begin();

		if(command.author->user_id == member->user_id) { // If for some reason you decided to kick yourself lol
			if(member->is_guild_owner()) {
				errors.emplace_back("❌ Why are you kicking yourself, server owner? lmfao");
				ignore_owner_repeat = true;
			}
			else {
				errors.emplace_back("❌ You can't kick yourself lmao.");
			}
			cancel_operation = true;
		}

		if(!ignore_owner_repeat && member->is_guild_owner()) {
			errors.emplace_back("❌ You can't kick the server owner lmfao.");
			cancel_operation = true;
		}


		if(command.bot->me.id == member->user_id) { // If you decided to kick the bot (ReactAIO)
			errors.emplace_back("❌ Can't kick myself lmfao.");
			cancel_operation = true;
		}

		if(!protected_roles.empty()) {

			shared_vector<dpp::role> member_protected_roles;
			reactaio::set_intersection(protected_roles, member_roles, member_protected_roles);

			if(!member_protected_roles.empty()) { // If member has any of the protected roles.
				cancel_operation = true;
				std::vector<std::string> role_mentions;
				std::ranges::transform(member_protected_roles, std::back_inserter(role_mentions), [](const role_ptr& role){
					return role->get_mention();
				});
				std::string role_mentions_str = join(role_mentions, " , ");
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot kick.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to kick. Please "
										 "move the bot role above the members and below your staff roles.",
										 member->get_mention()));
			cancel_operation = true;
		}

		if(member_top_role->position > author_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than you do. You can't kick them.",
										 member->get_mention()));
			cancel_operation = true;
		}
	}

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while kicking member(s): ")
								  .set_color(color::ERROR_COLOR)
								  .set_timestamp(time_now);
		for (auto const &error: organized_errors) {
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
		else {
			auto webhook_url_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			transaction.commit();
			auto const bot_error_webhook_url_field = webhook_url_query[0]["bot_error_logs"];
			if(!bot_error_webhook_url_field.is_null()) {
				auto bot_error_webhook_url = bot_error_webhook_url_field.as<std::string>();
				dpp::webhook bot_error_webhook{bot_error_webhook_url};
				command.bot->execute_webhook(bot_error_webhook, error_message);
			}
			else {
				error_message.set_content("This server hasn't set a channel for bot errors. So the errors are being "
										  "sent to your DMs:");
				command.bot->direct_message_create(command.author->user_id, error_message);
			}
		}
	}
}

void kick_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, member_ptr const &member) {
	if (completion.is_error()) {
		auto const error = completion.get_error();
		errors.emplace_back(std::format("❌ Unable to kick user **{}**. Error code {}: {}.", member->get_user()->format_username(), error.code, error.human_readable));
		members_with_errors.insert(member);
	}
	else {
		auto transaction = pqxx::work{*command.connection};
		auto const max_query	 = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
		auto max_id = std::get<0>(max_query.as<case_t>()) + 1;
		transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id,
								  reactaio::internal::mod_action_name::KICK, std::to_string(command.author->user_id),
								  std::to_string(member->user_id), command.reason);
		transaction.commit();
	}
}

void kick_wrapper::process_kicks() {
	for (auto const& member : members) {
		if (command.interaction) { // If this is automod, DMing lots of users WILL result in a ratelimit
			command.bot->direct_message_create(
				member->user_id,
				dpp::message(std::format("You have been kicked from {} by {}. Reason: {}.", command.guild->name,
										 member->get_user()->format_username(), command.reason)));
		}
		command.bot->set_audit_reason(std::format("Kicked by {} for reason: {}", command.author->get_user()->format_username(), command.reason)).guild_member_kick(command.guild->id, member->user_id, [this, member](auto const& completion) {
			lambda_callback(completion, member);
		});
	}
}

void kick_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author->get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed	= dpp::embed()
							  .set_title("Error while kicking member(s): ")
							  .set_color(color::ERROR_COLOR)

							  .set_timestamp(time_now);
		if(format_split.size() == 1) {
			base_embed.set_description(format_split[0]);
			message.add_embed(base_embed);
		}
		else {
			for (auto const& error : format_split) {
				auto embed{base_embed};
				embed.set_description(error);
				message.add_embed(embed);
			}
		}

		if (command.interaction) {
			if(are_all_errors())
				message.set_flags(dpp::m_ephemeral);// If there's no successful kick, no reason to show the errors publicly.
			if(format_split.size() == 1 && are_all_errors())
				(*command.interaction)->edit_response(message);
			else if(format_split.size() > 1 && are_all_errors()) {
				message_paginator paginator{message, command};
				paginator.start();
			}
		}
		else {
			// Get bot error webhook
			auto automod_log = dpp::message();
			auto transaction  = pqxx::work{*command.connection};
			auto webhook_url_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			transaction.commit();
			if(webhook_url_query.empty()) { // Bot has no error webhook set.
				automod_log.set_content("This server hasn't set a channel for bot errors. So the errors are being sent to your DMs:");
				for (auto const& error : format_split) {
					auto embed{base_embed};
					embed.set_description(error);
					automod_log.add_embed(embed);
				}
				command.bot->direct_message_create(command.author->user_id, automod_log);
			}
			else {
				auto webhook_url = webhook_url_query[0]["bot_error_logs"].as<std::string>();
				dpp::webhook automod_webhook{webhook_url};
				for (auto const& error : format_split) {
					auto embed{base_embed};
					embed.set_description(error);
					automod_log.add_embed(embed);
				}
				command.bot->execute_webhook(automod_webhook, automod_log);
			}

		}
	}
	if (!are_all_errors()) {
		shared_vector<dpp::guild_member> kicked_members;
		std::vector<std::string> kicked_usernames;
		std::vector<std::string> kicked_mentions;

		reactaio::set_difference(members, members_with_errors, kicked_members);

		reactaio::set_symmetric_difference(members, members_with_errors, kicked_members);


		std::ranges::transform(kicked_members, std::back_inserter(kicked_usernames), [](auto const& member) {
			return std::format("**{}**", member->get_user()->format_username());
		});

		std::ranges::transform(kicked_members, std::back_inserter(kicked_mentions), [](auto const& member) {
			return member->get_mention();
		});

		auto usernames = join(kicked_usernames, ", ");
		auto mentions  = join(kicked_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if (kicked_members.size() == 1) {
			title		= "Kicked";
			description = std::format("{} has been kicked.", usernames);
			gif_url		= "https://tenor.com/view/yeet-lion-king-simba-rafiki-throw-gif-16194362";
		}
		else {
			title		= "Mass kicked";
			description = std::format("{} have been kicked.", usernames);
			gif_url		= "https://tenor.com/view/the-simpsons-moe-homer-barney-moes-tavern-gif-17102369";
		}

		auto time_now	= std::time(nullptr);
		auto reason_str = std::string{command.reason};

		auto response = dpp::embed()
							.set_color(color::RESPONSE_COLOR)
							.set_title(title)
							.set_description(description)
							.set_image(gif_url)
							.set_timestamp(time_now)
							.set_footer(dpp::embed_footer().set_text(std::format("We're now at {} members.",
							                                                     non_bot_members(command.guild))));
		response.add_field("Moderator: ", author_user->get_mention());
		response.add_field("Reason: ", reason_str);

		message.add_embed(response);
	
		// Modlogs

		auto transaction = pqxx::work{*command.connection};
		auto query = transaction.exec_prepared("kick_modlog", std::to_string(command.guild->id));
		transaction.commit();
		auto const member_kick_webhook_url = query[0]["member_kick"];
		if(!member_kick_webhook_url.is_null()) {
			auto member_kick_webhook = dpp::webhook{member_kick_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(kicked_members.size() > 1)
				embed_title = "Members kicked: ";
			else {
				embed_title = "Member kicked: ";
				embed_image_url = kicked_members.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto kick_log = dpp::embed()
					.set_color(color::MEMBER_REMOVE_COLOR)
					.set_title(embed_title)
					.set_thumbnail(embed_image_url)
					.set_timestamp(time_now)
					.set_description(std::format("{} have been kicked.", usernames))
					.add_field("Moderator: ", command.author->get_mention())
					.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(kick_log);

			command.bot->execute_webhook(member_kick_webhook, log);
		}
		auto const modlog_webhook_url = query[0]["modlog"];
		if(!modlog_webhook_url.is_null()) {
			auto modlog_webhook = dpp::webhook{modlog_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(kicked_members.size() > 1)
				embed_title = "Members kicked: ";
			else {
				embed_title = "Member kicked: ";
				embed_image_url = kicked_members.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto kick_log = dpp::embed()
					.set_color(color::LOG_COLOR)
					.set_title(embed_title)
					.set_thumbnail(embed_image_url)
					.set_timestamp(time_now)
					.set_description(std::format("{} have been kicked.", usernames))
					.add_field("Moderator: ", command.author->get_mention())
					.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(kick_log);
			command.bot->execute_webhook(modlog_webhook, log);
		}
		auto public_modlog_webhook_url = query[0]["public_modlog"];
		if(!public_modlog_webhook_url.is_null()) {
			auto public_modlog_webhook = dpp::webhook{public_modlog_webhook_url.as<std::string>()};
			std::string embed_title = kicked_members.size() > 1 ? "Members kicked: " : "Member kicked: ";
			std::string embed_image_url = kicked_members.size() > 1 ? kicked_members.at(0)->get_avatar_url() : "";
			time_now = std::time(nullptr);
			auto kick_log = dpp::embed()
					.set_color(color::LOG_COLOR)
					.set_title(embed_title)
					.set_thumbnail(embed_image_url)
					.set_timestamp(time_now)
					.set_description(std::format("{} have been kicked.", usernames))
					.add_field("Moderator: ", command.author->get_mention())
					.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(kick_log);
			command.bot->execute_webhook(public_modlog_webhook, log);
		}
	}
	if (command.interaction) {
		if(message.embeds.size() > 2) {
			message_paginator paginator{message, command};
			paginator.start();
		}
		else
			(*command.interaction)->edit_response(message);
		// Log command call
		pqxx::work transaction{*command.connection};
		transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author->user_id),
														   reactaio::internal::mod_action_name::KICK, dpp::utility::current_date_time());
		transaction.commit();
	}
	else
		command.bot->message_create(message);
}
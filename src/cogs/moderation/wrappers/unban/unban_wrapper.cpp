//
// Created by arshia on 3/12/23.
//

#include "unban_wrapper.h"
#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../mod_action.h"


void unban_wrapper::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;

	process_unbans();
	process_response();
}

void unban_wrapper::check_permissions() {
	auto const* bot_user = &command.bot->me;
	auto const author_roles = get_roles_sorted(*command.author);
	auto const bot_member = find_guild_member(command.guild->id, bot_user->id);
	auto const& author_top_role = author_roles.front();
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();

	if(!bot_top_role->has_ban_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ The bot does not have ban_members permission. Please fix this and try again.");
	}

	if(author_top_role->position < bot_top_role->position) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot top role is below your role. Please move the bot role above the top role");
	}

	pqxx::work transaction{*command.connection};
	auto hard_bans_query = transaction.exec_prepared("hardban_get", std::to_string(command.guild->id));

	shared_vector<dpp::user> hard_bans;
	for(auto const& row: hard_bans_query)
		hard_bans.insert(std::make_shared<dpp::user>(*dpp::find_user(row["user_id"].as<snowflake_t>())));

	hard_bans.erase(nullptr); // Removing any chance of a null pointer.

	shared_vector<dpp::user> illegal_bans;
	reactaio::set_intersection(users, hard_bans, illegal_bans);

	if(!illegal_bans.empty() && !command.author->is_guild_owner()) {
		cancel_operation = true;
		users_with_errors.insert_range(illegal_bans);
		std::ranges::transform(illegal_bans, std::back_inserter(errors), [](user_ptr const& user){
			return std::format("User **{}** is hard banned by the server owner and can only be unbanned by said individual.", user->format_username());
		});
	}

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while unbanning user(s): ")
								  .set_color(ERROR_COLOR)
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
		else
			invoke_error_webhook();
	}
}

void unban_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, user_ptr const &user) {
	if(completion.is_error()) {
		auto error = completion.get_error();
		errors.emplace_back(std::format("❌ Error {}: {}", error.code, error.message));
		users_with_errors.insert(user);
		return;
	}
	pqxx::work transaction{*command.connection};
	auto max_query	 = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
	auto max_id = std::get<0>(max_query.as<case_t>()) + 1;
	transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id,
							  internal::mod_action_name::UNBAN, std::to_string(command.author->user_id), std::to_string(user->id),
							  command.reason);
	transaction.commit();
}

void unban_wrapper::process_unbans() {
	for(auto const& user: users) {
		command.bot->set_audit_reason(std::format("Unbanned by {} for reason: {}", command.author->get_user()->format_username(), command.reason)).guild_ban_delete(command.guild->id, user->id, [this, user](auto const& completion){
			lambda_callback(completion, user);
		});
	}
}

void unban_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author->get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed	= dpp::embed()
								  .set_title("Error while unbanning user(s): ")
								  .set_color(ERROR_COLOR)

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
		else
			invoke_error_webhook();
	}
	if (!are_all_errors()) {
		shared_vector<dpp::user> unbanned_users;
		std::vector<std::string> unbanned_usernames;
		std::vector<std::string> unbanned_mentions;

		reactaio::set_difference(users, users_with_errors, unbanned_users);

		std::ranges::transform(unbanned_users, std::back_inserter(unbanned_usernames), [](user_ptr const& user){
			return std::format("**{}**", user->username);
		});

		std::ranges::transform(unbanned_users, std::back_inserter(unbanned_mentions), [](user_ptr const& user) {
			return user->get_mention();
		});

		auto usernames = join(unbanned_usernames, ", ");
		auto mentions  = join(unbanned_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if (unbanned_users.size() == 1) {
			title		= "Unbanned";
			description = std::format("{} has been unbanned.", usernames);
			gif_url		= "https://tenor.com/view/yeet-lion-king-simba-rafiki-throw-gif-16194362";
		}
		else {
			title		= "Mass unbanned";
			description = std::format("{} have been unbanned.", usernames);
			gif_url		= "https://tenor.com/view/the-simpsons-moe-homer-barney-moes-tavern-gif-17102369";
		}

		auto time_now	= std::time(nullptr);
		auto reason_str = std::string{command.reason};

		auto response = dpp::embed()
								.set_color(RESPONSE_COLOR)
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
		auto query = transaction.exec_prepared1("get_unban_log", std::to_string(command.guild->id));
		transaction.commit();
		auto member_kick_webhook_url = query["member_ban_remove"];
		if(!member_kick_webhook_url.is_null()) {
			auto member_kick_webhook = dpp::webhook{member_kick_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(unbanned_users.size() > 1)
				embed_title = "Members unbanned: ";
			else {
				embed_title = "Member unbanned: ";
				embed_image_url = unbanned_users.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto kick_log = dpp::embed()
									.set_color(LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unbanned.", usernames))
									.add_field("Moderator: ", command.author->get_mention())
									.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(kick_log);

			command.bot->execute_webhook(member_kick_webhook, log);
		}
		auto modlog_webhook_url = query["modlog"];
		if(!modlog_webhook_url.is_null()) {
			auto modlog_webhook = dpp::webhook{modlog_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(unbanned_users.size() > 1)
				embed_title = "Users unbanned: ";
			else {
				embed_title = "User unbanned: ";
				embed_image_url = unbanned_users.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto kick_log = dpp::embed()
									.set_color(UNBAN_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unbanned.", usernames))
									.add_field("Moderator: ", command.author->get_mention())
									.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(kick_log);
			command.bot->execute_webhook(modlog_webhook, log);
		}
		auto public_modlog_webhook_url = query["public_modlog"];
		if(!public_modlog_webhook_url.is_null()) {
			auto public_modlog_webhook = dpp::webhook{public_modlog_webhook_url.as<std::string>()};
			std::string embed_title = unbanned_users.size() > 1 ? "Users unbanned: " : "User unbanned: ";
			std::string embed_image_url = unbanned_users.size() > 1 ? unbanned_users.at(0)->get_avatar_url() : "";
			time_now = std::time(nullptr);
			auto unban_log = dpp::embed()
									.set_color(LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unbanned.", usernames))
									.add_field("Moderator: ", command.author->get_mention())
									.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(unban_log);
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
		log_command_invoke(internal::mod_action_name::UNBAN);
	}
	else
		command.bot->message_create(message);

}
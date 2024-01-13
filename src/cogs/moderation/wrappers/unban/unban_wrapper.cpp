//
// Created by arshia on 3/12/23.
//

#include <unordered_set>

#include "unban_wrapper.h"
#include "../../../base/consts.h"
#include "../../../base/helpers.h"
#include "../../../base/colors.h"
#include "../../../base/datatypes/message_paginator.h"
#include "../../mod_action.h"



void unban_wrapper::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;

	process_unbans();
	process_response();
}

void unban_wrapper::check_permissions() {
	auto* bot_user = &command.bot->me;
	auto author_roles = get_member_roles_sorted(command.author);
	auto bot_member = dpp::find_guild_member(command.guild->id, bot_user->id);
	auto* author_top_role = *author_roles.begin();
	auto bot_roles = get_member_roles_sorted(bot_member);
	auto* bot_top_role = *bot_roles.begin();

	if(!bot_top_role->has_ban_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ The bot does not have ban_members permission. Please fix this and try again.");
	}

	if(author_top_role->position < bot_top_role->position) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot top role is below your role. Please move the bot role above the top role");
	}

	auto transaction = pqxx::transaction{*command.connection};
	auto hard_bans_query = transaction.exec_prepared("hardban_get", std::to_string(command.guild->id));

	std::unordered_set<dpp::user*> hard_bans;
	for(auto const& row: hard_bans_query)
		hard_bans.insert(dpp::find_user(row["user_id"].as<snowflake_t>()));

	hard_bans.erase(nullptr); // Removing any chance of a null pointer.

	std::vector<dpp::user*> illegal_bans;
	std::ranges::copy_if(users, std::back_inserter(illegal_bans), [hard_bans](dpp::user* user){
		return hard_bans.contains(user);
	});

	if(!illegal_bans.empty() && command.author.user_id != command.guild->owner_id) {
		cancel_operation = true;
		users_with_errors.insert(users_with_errors.end(), illegal_bans.begin(), illegal_bans.end());
		std::ranges::transform(illegal_bans, std::back_inserter(errors), [](dpp::user* user){
			return std::format("User **{}** is hard banned by the server owner and can only be unbanned by said individual.", user->format_username());
		});
	}

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while unbanning user(s): ")
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
				command.interaction->edit_response(error_message);
			else {
				message_paginator paginator{error_message, command};
				paginator.start();
			}
		}
		else {
			auto webhook_url_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			transaction.commit();
			auto bot_error_webhook_url_field = webhook_url_query[0]["bot_error_logs"];
			if (!bot_error_webhook_url_field.is_null()) {
				auto bot_error_webhook_url = bot_error_webhook_url_field.as<std::string>();
				dpp::webhook bot_error_webhook{bot_error_webhook_url};
				command.bot->execute_webhook(bot_error_webhook, error_message);
			} else {
				error_message.set_content("This server hasn't set a channel for bot errors. So the errors are being "
										  "sent to your DMs:");
				command.bot->direct_message_create(command.author.user_id, error_message);
			}
		}
	}
}

void unban_wrapper::process_unbans() {
	for(auto* user: users) {
		command.bot->set_audit_reason(std::string(command.reason)).guild_ban_delete(command.guild->id, user->id, [this, user](auto const& completion){
			if(completion.is_error()) {
				auto error = completion.get_error();
				errors.emplace_back(std::format("❌ Error {}: {}", error.code, error.message));
				users_with_errors.push_back(user);
				return;
			}
			auto transaction = pqxx::transaction{*command.connection};
			auto max_query	 = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
			auto max_id = std::get<0>(max_query.as<case_t>()) + 1;
			transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id,
									  reactaio::internal::mod_action_name["unban"], std::to_string(command.author.user_id), std::to_string(user->id),
									  command.reason);
			transaction.commit();
		});
	}
}

void unban_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author.get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed	= dpp::embed()
								  .set_title("Error while unbanning user(s): ")
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
				command.interaction->edit_response(message);
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
				command.bot->direct_message_create(command.author.user_id, automod_log);
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
		std::vector<dpp::user*> unbanned_users;
		std::vector<std::string> unbanned_usernames;
		std::vector<std::string> unbanned_mentions;

		std::ranges::copy_if(users, std::back_inserter(unbanned_users), [this](dpp::user* user){
			return !includes(users_with_errors, user);
		});

		std::ranges::transform(unbanned_users, std::back_inserter(unbanned_usernames), [](dpp::user* user){
			return std::format("**{}**", user->username);
		});

		std::ranges::transform(unbanned_users, std::back_inserter(unbanned_mentions), [](dpp::user* user) {
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
									.set_color(color::LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unbanned.", usernames))
									.add_field("Moderator: ", command.author.get_mention())
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
									.set_color(color::UNBAN_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unbanned.", usernames))
									.add_field("Moderator: ", command.author.get_mention())
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
			auto kick_log = dpp::embed()
									.set_color(color::LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unbanned.", usernames))
									.add_field("Moderator: ", command.author.get_mention())
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
			command.interaction->edit_response(message);
		// Log command call
		pqxx::work transaction{*command.connection};
		transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author.user_id),
								  reactaio::internal::mod_action_name["kick"], dpp::utility::current_date_time());
		transaction.commit();
	}
	else
		command.bot->message_create(message);

}
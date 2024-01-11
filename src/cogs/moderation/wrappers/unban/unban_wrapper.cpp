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

}
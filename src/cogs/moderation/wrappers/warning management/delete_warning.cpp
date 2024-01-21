//
// Created by arshia on 1/20/24.
//

#include "delete_warning.h"
#include "../../../core/helpers.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"

void delete_warning::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void delete_warning::check_permissions() {
	auto bot_user = command.bot->me;
	auto bot_member = dpp::find_guild_member(command.guild->id, bot_user.id);
	auto bot_roles_sorted = get_roles_sorted(bot_member);
	auto* bot_top_role = *bot_roles_sorted.begin();

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot has Moderate Members permission.");
	}

	// Get member from warning id
	pqxx::work transaction{*command.connection};
	auto query = transaction.exec_prepared1("warning_lookup", warning_id, std::to_string(command.guild->id));
	transaction.commit();
	if(!query["user_id"].is_null()) {
		member = dpp::find_guild_member(command.guild->id, query["user_id"].as<snowflake_t>());
		auto member_roles_sorted = get_roles_sorted(member);
		auto* member_top_role = *member_roles_sorted.begin();

		if(member_top_role->position > bot_top_role->position) {
			cancel_operation = true;
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to delete a warning for this member. Please "
										 "move the bot role above the members and below your staff roles.", member.get_mention()));

		}

		//TODO Check author permissions that if it either has Moderate Member perm or has a custom role set as the allowed role.

		return;
	}
	cancel_operation = true;
	errors.emplace_back("❌  Warning does not exist.");
}

void delete_warning::process_response() {
	if(are_all_errors()) {
		auto split_format = join_with_limit(errors, bot_max_embed_chars);
		if(split_format.size() == 1) {
			response = dpp::message{command.channel_id, split_format.front()}.set_flags(dpp::m_ephemeral);
			if(command.interaction) { // Will always be true but failsafe
				command.interaction->edit_response(response);
				return;
			}
		}
		else {
			response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
			message_paginator paginator{response, errors, command};
			paginator.start();
		}
		return;
	}
	pqxx::work transaction{*command.connection};
	auto result = transaction.exec_prepared1("remove_warning", warning_id, std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result["warn_id"].is_null()) {
		auto time_now = std::time(nullptr);
		auto embed = dpp::embed()
							 .set_color(color::INFO_COLOR)
							 .set_title(std::format("Warning {} for member **{}** has been removed.", warning_id, member.get_user()->format_username()))
							 .set_timestamp(time_now)
							 .set_footer(dpp::embed_footer().set_text(std::format("User id {}", std::to_string(member.user_id))));
		response.add_embed(embed);
		if(command.interaction) { // Will always be true but failsafe
			command.interaction->edit_response(response);
			return;
		}
	}
	if(command.interaction) // Will always be true but failsafe.
		command.interaction->edit_response("Warning doesn't exist.");
}

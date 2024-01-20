//
// Created by arshia on 1/20/24.
//

#include "clear_warnings.h"
#include "../../../core/helpers.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"


void clear_warnings::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}
void clear_warnings::check_permissions() {
	auto bot_user = command.bot->me;
	auto bot_member = dpp::find_guild_member(command.guild->id, bot_user.id);
	auto bot_roles_sorted = get_roles_sorted(bot_member);
	auto member_roles_sorted = get_roles_sorted(member);
	auto* bot_top_role = *bot_roles_sorted.begin();
	auto* member_top_role = *member_roles_sorted.begin();

	if(member_top_role->position > bot_top_role->position) {
		cancel_operation = true;
		errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to clear warnings for this member. Please "
									 "move the bot role above the members and below your staff roles.", member.get_mention()));

	}

	//TODO Check author permissions that if it either has Moderate Member perm or has a custom role set as the allowed role.
}

void clear_warnings::process_response() {
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
	auto result = transaction.exec_prepared1("clear_warnings", std::to_string(member.user_id), std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result["user_id"].is_null()) {
		auto time_now = std::time(nullptr);
		auto embed = dpp::embed()
							 .set_color(color::INFO_COLOR)
							 .set_title(std::format("Warnings for member **{}** cleared.", member.get_user()->format_username()))
							 .set_timestamp(time_now)
							 .set_footer(dpp::embed_footer().set_text(std::format("User id {}", std::to_string(member.user_id))));
		response.add_embed(embed);
		if(command.interaction) { // Will always be true but failsafe
			command.interaction->edit_response(response);
			return;
		}
	}
	if(command.interaction) // Will always be true but failsafe.
		command.interaction->edit_response("This member has no warnings.");
}

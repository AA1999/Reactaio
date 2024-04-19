//
// Created by arshia on 1/19/24.
//

#include "view_warnings.h"

#include "../../../core/helpers.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"

void view_warnings::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void view_warnings::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = dpp::find_guild_member(command.guild->id, bot_user.id);
	auto const bot_roles_sorted = get_roles_sorted(bot_member);
	auto const member_roles_sorted = get_roles_sorted(*member);
	auto const& bot_top_role = bot_roles_sorted.front();
	auto const& member_top_role = member_roles_sorted.front();

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Moderate Members permission.");
	}

	if(member_top_role->position > bot_top_role->position) {
		cancel_operation = true;
		errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to view warnings for this member. Please "
									 "move the bot role above the members and below your staff roles.", member->get_mention()));

	}

	//TODO Check author permissions that if it either has Moderate Member perm or has a custom role set as the allowed role.
}

void view_warnings::process_response() {
	if(are_all_errors()) {
		auto split_format = join_with_limit(errors, bot_max_embed_chars);
		if(split_format.size() == 1) {
			response = dpp::message{command.channel_id, split_format.front()}.set_flags(dpp::m_ephemeral);
			if(command.interaction) { // Will always be true but failsafe
				(*command.interaction)->edit_response(response);
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
	auto const result = transaction.exec_prepared("view_warnings", std::to_string(command.guild->id), std::to_string(member->user_id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result.empty()) {
		auto const time_now = std::time(nullptr);
		auto embed = dpp::embed()
								.set_color(color::INFO_COLOR)
								.set_title(std::format("Warnings for member **{}**", member->get_user()->format_username()))
								.set_timestamp(time_now)
								.set_footer(dpp::embed_footer().set_text(std::format("User id {}", std::to_string(member->user_id))));
		for(auto const& row: result)
			embed.add_field(std::format("Warning id {}", row["warn_id"].as<std::string>()), std::format("Reason: {}", row["reason"].as<std::string>()));
		response.add_embed(embed);
		if(command.interaction) { // Will always be true but failsafe
			(*command.interaction)->edit_response(response);
			return;
		}
	}
	if(command.interaction) // Will always be true but failsafe.
		(*command.interaction)->edit_response("This member has no warnings.");
}

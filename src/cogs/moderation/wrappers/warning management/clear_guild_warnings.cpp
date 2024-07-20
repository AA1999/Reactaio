//
// Created by arshia on 1/20/24.
//

#include "clear_guild_warnings.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"

void clear_guild_warnings::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void clear_guild_warnings::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = dpp::find_guild_member(command.guild->id , bot_user.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();

	if(!bot_top_role->has_manage_guild()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Manage Server permission.");
	}

	//TODO check if author has permission to do this by either specified role or Moderate Members perm
}

void clear_guild_warnings::process_response() {
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
	auto result = transaction.exec_prepared1("clear_guild_warnings", std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result["guild_id"].is_null()) {
		auto time_now = std::time(nullptr);
		auto embed = dpp::embed()
							 .set_color(color::INFO_COLOR)
							 .set_title("Warnings for this server have been removed.")
							 .set_timestamp(time_now)
							 .set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));

		response.add_embed(embed);
		if(command.interaction) { // Will always be true but failsafe
			(*command.interaction)->edit_response(response);
			return;
		}
	}
	if(command.interaction) // Will always be true but failsafe.
		(*command.interaction)->edit_response("This guild has no warnings."); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

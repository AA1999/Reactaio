//
// Created by arshia on 2/28/24.
//

#include "view_guild_warnings.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"

void view_guild_warnings::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void view_guild_warnings::check_permissions() {
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

void view_guild_warnings::process_response() {
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
	auto const result = transaction.exec_prepared("view_guild_warnings", std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result.empty()) {
		auto const time_now = std::time(nullptr);
		std::vector<dpp::embed> embeds;
		auto const embed_template = dpp::embed()
							 .set_color(color::INFO_COLOR)
							 .set_title(std::format("Warnings for {}", command.guild->name))
							 .set_timestamp(time_now);
		for(auto const& row: result) {
			auto embed{embed_template};
			auto const user_id = row["user_id"].as<snowflake_t>();
			auto const warning_id = row["warn_id"].as<warn_t>();
			auto const mod_id = row["mod_id"].as<snowflake_t>();
			auto const reason = row["reason"].as<std::string>();
			auto const* user = dpp::find_user(user_id);
			auto const mod = dpp::find_guild_member(command.guild->id, mod_id);
			embed.add_field("Warning id: ", std::to_string(warning_id));
			embed.add_field("User: ", std::format("**{}**", user->format_username()), true);
			embed.add_field("Warned by: ", std::format("**{}**", mod.get_user()->format_username()), true);
			embed.add_field("Reason: ", reason, true);
			embeds.push_back(embed);
			response.add_embed(embed);
		}
		if(embeds.size() > bot_max_embeds) {
			message_paginator paginator{response, command};
			paginator.start();
			return;
		}
		if(command.interaction)
			(*command.interaction)->edit_response(response);
		return;;
	}
	if(command.interaction) // Will always be true but failsafe.
		(*command.interaction)->edit_response("This server has no warnings.");
}

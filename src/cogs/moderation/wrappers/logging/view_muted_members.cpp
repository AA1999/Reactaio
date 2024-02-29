//
// Created by arshia on 2/29/24.
//

#include "view_muted_members.h"
#include "../../../core/consts.h"
#include "../../../core/colors.h"
#include "../../../core/helpers.h"
#include "../../../core/datatypes/message_paginator.h"

#include <algorithm>

void view_muted_members::wrapper_function() {
	check_permissions();
	if(!cancel_operation)
		process_response();
}

void view_muted_members::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = dpp::find_guild_member(command.guild->id , bot_user.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const* bot_top_role = *bot_roles.begin();

	if(!bot_top_role->has_manage_guild()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Manage Guild permission.");
	}
}

void view_muted_members::process_response() {
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
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	pqxx::work transaction{*command.connection};
	auto result = transaction.exec_prepared("view_guild_muted_members", std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result.empty()) {
		auto const time_now = std::time(nullptr);
		auto const embed_template = dpp::embed()
							 .set_color(color::INFO_COLOR)
							 .set_timestamp(time_now)
							 .set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));
		for(auto const row: result) {
			auto const user_id{row["punished_id"].as<snowflake_t>()};
			auto const mod_id{row["mod_id"].as<snowflake_t>()};
			auto const case_id{row["case_id"].as<case_t>()};
			auto const reason{row["reason"].as<std::string>()};
			auto const duration_str{row["duration"].as<std::string>()};
			auto const punishment_type{row["action"].as<std::string>()};
			auto embed{embed_template};
			auto const member = dpp::find_guild_member(command.guild->id, user_id);
			auto const mod = dpp::find_guild_member(command.guild->id, mod_id);
			embed.set_title(std::vformat("Case #{}: ", std::make_format_args(case_id)));
			embed.add_field("User: ", member.get_user()->username, true);
			embed.add_field("Punishment: ", punishment_type, true);
			embed.add_field("Reason: ", reason, true);
			embed.add_field("Duration: ", duration_str, true);
			response.add_embed(embed);
		}
		if(response.embeds.size() > bot_max_embeds) {
			message_paginator paginator{response, command};
			paginator.start();
		}
		else
			command.interaction->edit_response(response);
		if(command.interaction) { // Will always be true but failsafe
			command.interaction->edit_response(response);
			return;
		}
		return;
	}
	if(command.interaction) // Will always be true but failsafe.
		command.interaction->edit_response("This guild has no muted/timed out members."); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

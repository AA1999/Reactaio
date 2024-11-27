//
// Created by arshia on 9/21/24.
//

#include "view_mod_modcases.h"

#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"

void view_mod_modcases::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = find_guild_member(command.guild->id , bot_user.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Moderate Members permission.");
	}

	//TODO check if author has permission to do this by either specified role or Moderate Members perm
}

void view_mod_modcases::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void view_mod_modcases::process_response() {
	if(are_all_errors()) {
		auto const split_format = join_with_limit(errors, bot_max_embed_chars);
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
	const auto result = transaction.exec_prepared("modcase_view_user", mod->user_id, std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result.empty()) {
		auto const time_now = std::time(nullptr);
		auto embed = dpp::embed()
							.set_color(INFO_COLOR)
							.set_title(std::format("Mod: {}", mod->user_id))
							.set_timestamp(time_now)
							.set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));
		for(auto const& row: result) {
			auto const mod_id = row["mod_id"].as<dpp::snowflake>();
			auto const reason = row["reason"].as<std::string>();
			auto const action = row["action"].as<std::string>();
			auto const punished_id = row["punished_id"].as<dpp::snowflake>();
			try {
				auto const punished_member = find_guild_member(punished_id, command.guild->id);
				embed.add_field("User: ", punished_member.get_mention());
			}
			catch (dpp::exception&) {

			}
			embed.add_field("Punishment: ", action, true);
			embed.add_field("Punished By: ", mod->get_mention(), true);
			if(!row["duration"].is_null()) {
				auto const duration = row["duration"].as<std::string>();
				embed.add_field("Duration: ", duration, true);
			}
			embed.add_field("Reason: ", reason, true);
		}
		if(command.interaction) {
			// Will always be true but failsafe
			(*command.interaction)->edit_response(response);
			return;
		}
	}
	if(command.interaction) // Will always be true but failsafe.
		command.interaction.value()->edit_response(std::format("No modcase found for mod **{}**.", mod->get_user()->format_username())); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

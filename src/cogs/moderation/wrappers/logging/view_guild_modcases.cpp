//
// Created by arshia on 9/21/24.
//

#include "view_guild_modcases.h"

#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"


void view_guild_modcases::check_permissions() {
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


void view_guild_modcases::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}


void view_guild_modcases::process_response() {
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
	const auto result = transaction.exec_prepared("modcase_view_guild", command.guild->id, std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result.empty()) {
		auto const time_now = std::time(nullptr);
		std::vector<dpp::embed> embeds;
		auto template_embed = dpp::embed()
							.set_color(INFO_COLOR)
							.set_title(std::format("All modcases for {}", command.guild->name))
							.set_timestamp(time_now)
							.set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));
		for(auto const& row: result) {
			auto const mod_id = row["mod_id"].as<dpp::snowflake>();
			auto const mod = find_guild_member(mod_id, command.guild->id);
			auto const reason = row["reason"].as<std::string>();
			auto const action = row["action"].as<std::string>();
			auto const user_id = row["punished_id"].as<dpp::snowflake>();
			auto embed{template_embed};
			try {
				auto const member = find_guild_member(command.guild->id, user_id);
				embed.add_field("User: ", member.get_mention());
				embed.add_field("User: ", member.get_user()->format_username());
			}
			catch(dpp::cache_exception&) {
				continue;
			}
			embed.add_field("User id: ", std::to_string(user_id));
			embed.add_field("Punishment: ", action, true);
			embed.add_field("Punished By: ", mod.get_mention(), true);
			if(!row["duration"].is_null()) {
				auto const duration = row["duration"].as<std::string>();
				embed.add_field("Duration: ", duration, true);
			}
			embed.add_field("Reason: ", reason, true);
			embeds.push_back(embed);
		}
		if(embeds.size() != 1) {
			response = dpp::message{command.channel_id, ""};
			for(auto const& embed: embeds)
				response.add_embed(embed);
			message_paginator paginator{response, command};
			paginator.start();
		}
		if(command.interaction) {
			// Will always be true but failsafe
			(*command.interaction)->edit_response(response);
			return;
		}
	}
	if(command.interaction) // Will always be true but failsafe.
		command.interaction.value()->edit_response("No modcase found for this guild."); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

//
// Created by arshia on 7/15/24.
//

#pragma once

#include "slash_command_properties.h"

namespace reactaio::moderation {
	/**
	 * @brief Creates a global discord command for the bot.
	 * @param properties Properties of the slash command: name, description and a pointer to the bot.
	 * @param options A list of command options.
	 * @return The resulting slash command.
	 */
	inline dpp::slashcommand define_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options);
	inline void define_guild_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options, dpp::snowflake const &guild_id);
}
//
// Created by arshia on 7/15/24.
//

#pragma once

#include "slash_command_properties.h"

namespace reactaio::internal {

	/**
	 * @brief Creates a global discord command for the bot.
	 * @param properties A struct wrapping the constructor for dpp::slash_command.
	 * @param options A list of command options.
	 * @return The resulting slash command.
	 */
	inline dpp::slashcommand define_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options);


	/**
	 * @brief Creates a guild discord command.
	 * @param properties A struct wrapping the constructor for dpp::slash_command.
	 * @param options A list of command options.
	 * @param guild_id The guild to register the command to.
	 */
	inline void define_guild_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options, dpp::snowflake const &guild_id);
}
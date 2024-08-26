//
// Created by arshia on 7/15/24.
//

#pragma once

#include "../../core/discord/slash_command_properties.h"
#include "../modules/moderation.h"


namespace reactaio::moderation {
	inline dpp::slashcommand define_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options);
	inline void define_guild_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options, dpp::snowflake const &guild_id);
}
//
// Created by arshia on 1/15/24.
//

#pragma once

#include "command_wrapper.h"

/**
 * @brief guild_wrapper - Wrapper used to handle operations on a specific guild.
 */
class guild_wrapper: public command_wrapper {
	dpp::guild* guild;

public:
	guild_wrapper() = delete;
	~guild_wrapper() override = default;
	explicit guild_wrapper(moderation_command& command): command_wrapper(std::move(command)), guild(this->command.guild){};
};

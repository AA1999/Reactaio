//
// Created by arshia on 11/6/23.
//

#pragma once
#include "../../modules/moderation_command.h"
#include "ban_processor.h"

/**
 * @brief guild_bans_wrapper - A wrapper used for getting all the bans from a guild.
 * @note This was originally inherited from user_wrapper but for a lot of reasons like no requirement for a message the class was changed
 */

class guild_bans_wrapper {
	ban_vector bans;
	moderation_command command;
	std::vector<std::string> errors;

	bool are_errors{false};

	void get_all_guild_bans(dpp::snowflake after = 1);
public:
	explicit guild_bans_wrapper(moderation_command& command): command(std::move(command)){
		get_all_guild_bans();
	}

	~guild_bans_wrapper() = default;

	ban_vector guild_bans() const;
	std::vector<std::string> what() const;
	bool is_error() const;

};

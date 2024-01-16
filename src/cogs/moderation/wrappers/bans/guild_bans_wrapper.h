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

	/**
	 * @brief get_all_guild_bans - Fetches all the bans from a guild recursively.
	 * @param after The snowflake matching the search. Defaults to 1 because all snowflakes will certainly be bigger than 1.
	 */
	void get_all_guild_bans(dpp::snowflake after = 1);
public:

	/**
	 * @brief The constructor used to get the ban data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit guild_bans_wrapper(moderation_command& command): command(std::move(command)){
		get_all_guild_bans();
	}

	virtual ~guild_bans_wrapper() = default;

	/**
	 * @brief guild_bans - Returns all the guild bans.
	 * @return A vector of all the bans in the guild.
	 */
	ban_vector guild_bans() const;

	/**
	 * @brief what - Returns all the errors caught in the process.
	 * @return All the errors in the process.
	 */
	std::vector<std::string> what() const;

	/**
	 * @brief has_error - Checks if there was any error in the operation.
	 * @return true if any errors occured, false otherwise.
	 */
	bool is_error() const;

};

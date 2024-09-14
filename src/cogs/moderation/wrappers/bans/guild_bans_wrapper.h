//
// Created by arshia on 11/6/23.
//

#pragma once

#include "../../wrappers/recursive_wrapper.h"

/**
 * @brief A wrapper used for getting all the bans from a guild.
 */
class guild_bans_wrapper final: public recursive_wrapper {
	shared_vector<dpp::ban> bans;
	std::vector<std::string> banned_usernames;

	/**
	 * @brief Fetches all the bans from a guild recursively.
	 * @param after The snowflake matching the search. Defaults to 1 because all snowflakes will certainly be bigger than 1.
	 */
	void recursive_call(dpp::snowflake after = 1) override;

	/**
	 * 	@brief Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * 	@brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();


public:
	~guild_bans_wrapper() override = default;
	guild_bans_wrapper() = delete;
	using recursive_wrapper::recursive_wrapper;
};
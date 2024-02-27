//
// Created by arshia on 11/6/23.
//

#pragma once

#include "../../wrappers/guild_wrapper.h"

#include <unordered_map>

/**
 * @brief guild_bans_wrapper - A wrapper used for getting all the bans from a guild.
 */
class guild_bans_wrapper: public guild_wrapper {
	std::vector<dpp::ban> bans;
	std::vector<std::string> banned_usernames;

	/**
	 * @brief get_all_guild_bans - Fetches all the bans from a guild recursively.
	 * @param after The snowflake matching the search. Defaults to 1 because all snowflakes will certainly be bigger than 1.
	 */
	void get_all_guild_bans(dpp::snowflake after = 1);

	/**
	 * 	@brief check_permissions - Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * 	@brief wrapper_function - The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();


	void lambda_callback([[maybe_unused]] dpp::confirmation_callback_t const& completion, [[maybe_unused]] dpp::user* user) override {}

public:
	~guild_bans_wrapper() override = default;
	guild_bans_wrapper() = delete;
	using guild_wrapper::guild_wrapper;
};
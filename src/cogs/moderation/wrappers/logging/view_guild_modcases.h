//
// Created by arshia on 9/21/24.
//

#pragma once
#include "../simple_wrapper.h"

/**
 * @brief Wrapper to manage viewing modcases of a guild.
 */
class view_guild_modcases final : public simple_wrapper {
	dpp::message response;
public:
	/**
	 * @brief Constructs a new instance based on the command wrapper.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	explicit view_guild_modcases(moderation_command command): simple_wrapper(std::move(command)) {}

	/**
	 * @brief Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * @brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief Shows all the modcases for the user.
	 */
	void process_response();
};

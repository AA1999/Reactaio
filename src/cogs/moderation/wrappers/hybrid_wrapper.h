//
// Created by arshia on 3/12/23.
//

#pragma once

#include "command_wrapper.h"

#include <dpp/dpp.h>
#include <variant>
#include <vector>

/**
 * @brief hybrid_wrapper - A wrapper used to process commands with a mix of dpp::user* and dpp::guild_member elements.
 */

class hybrid_wrapper: public command_wrapper {
protected:
	std::vector<std::variant<dpp::guild_member, dpp::user*>> snowflakes;
	std::vector<dpp::user*> users_with_errors;
	std::vector<dpp::user*> users;
	std::vector<dpp::guild_member> members;

	/**
	 * 	@brief check_permissions - Checks if the user issuing the wrapper has the sufficient permission.
	 * 	@note This is an abstract function.
	 */
	void check_permissions() override = 0;

	/**
	 * 	@brief wrapper_function - The main function that manages every internal working of the wrapper and the three processes that are performed.
	 * 	@note This is an abstract function.
	 */
	void wrapper_function() override = 0;

	/**
	 * @brief lambda_callback - This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param user User object that the callback is made on.
	 */
	virtual void lambda_callback(dpp::confirmation_callback_t const& completion, dpp::user* user) = 0;

public:

	~hybrid_wrapper() override = default;

	/**
	 * @brief are_all_errors - Checks if every item has encountered an error.
	 * @return Whether if every single item given to the wrapper has encountered an error.
	 */
	[[nodiscard]] constexpr bool are_all_errors() const override {
		return users_with_errors.size() == snowflakes.size();
	}

	/**
	 * @brief The main constructor of the class used to get data from the command.
	 * @param snowflakes The list of dpp::user* and dpp::guild_member objects.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	hybrid_wrapper(const std::vector<std::variant<dpp::guild_member, dpp::user *>> &snowflakes, moderation_command& command)
		: command_wrapper(std::move(command)), snowflakes(snowflakes) {}

};

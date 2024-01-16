//
// Created by arshia on 3/12/23.
//

#pragma once

#include "command_wrapper.h"

#include <dpp/dpp.h>
#include <vector>

/**
 * @brief member_wrapper - A wrapper used to process commands with only dpp::guild_member elements.
 */
class member_wrapper: public command_wrapper {
protected:
	std::vector<dpp::guild_member> members;
	std::vector<dpp::guild_member> members_with_errors;

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
	 * @param member Member object that the callback is made on.
	 */
	virtual void lambda_callback(dpp::confirmation_callback_t const& completion, [[maybe_unused]] dpp::guild_member const& member) = 0;

public:
	~member_wrapper() override = default;


	/**
	 * @brief The main constructor of the class used to fetch data from the
	 * @param members The list of the guild members
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	member_wrapper(const std::vector<dpp::guild_member>& members, moderation_command& command): members(members), command_wrapper(std::move(command)){}

	/**
	 * @brief are_all_errors - Checks if every item has encountered an error.
	 * @return Whether if every single item given to the wrapper has encountered an error.
	 */
	[[nodiscard]] bool are_all_errors() const override;
};

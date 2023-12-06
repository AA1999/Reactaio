//
// Created by arshia on 3/12/23.
//

#pragma once

#include "base_wrapper.h"

#include <dpp/dpp.h>
#include <vector>

/**
 * @brief member_wrapper - A wrapper used to process commands with only dpp::guild_member elements.
 */
class member_wrapper: public base_wrapper {
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

public:
	~member_wrapper() override = 0;

	member_wrapper(const std::vector<dpp::guild_member>& members, moderation_command& command): members(members), base_wrapper(std::move(command)){}

	/**
	 * @brief are_all_errors - Checks if every item has encountered an error.
	 * @return Whether if every single item given to the wrapper has encountered an error.
	 */
	[[nodiscard]] bool are_all_errors() const override;
};

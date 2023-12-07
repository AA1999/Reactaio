//
// Created by arshia on 3/12/23.
//

#pragma once

#include "base_wrapper.h"

/**
 * @brief hybrid_wrapper - A wrapper used to process commands of dpp::user* elements.
 */
class user_wrapper : public base_wrapper {
protected:
	std::vector<dpp::user*> users;
	std::vector<dpp::user*> users_with_errors;

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
	 * @brief are_all_errors - Checks if every item has encountered an error.
	 * @return Whether if every single item given to the wrapper has encountered an error.
	 */
	[[nodiscard]] bool are_all_errors() const override;

public:
	user_wrapper(const std::vector<dpp::user*>& users, moderation_command& command): users(users), base_wrapper(std::move(command)){}

	~user_wrapper() = 0;
};

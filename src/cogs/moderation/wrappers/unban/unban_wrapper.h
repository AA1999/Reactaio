//
// Created by arshia on 3/12/23.
//

#pragma once

#include "../../../base/datatypes/ban.h"
#include "../user_wrapper.h"
#include <vector>

/**
 * @brief unban_wrapper - Functor that processes the unban operation for a list of dpp::user* objects.
 */
class unban_wrapper: public user_wrapper {
	/**
	 * @brief wrapper_function - Function called when the () operator is called. Unbans all the given members/users from the guild and returns any errors/success message.
	 */
	void wrapper_function() override;

	/**
	 * @brief check_permissions - Checks if the command invoker has the permission to unban anyone from the guild (including the bot).
	 * @note This function works internally and returns all error to the wrapper_function() function.
	 */
	void check_permissions() override;

	/**
	 * @brief process_unbans - Unbans the list of users/members from the guild and returns errors/success message.
	 * @note This function works internally and returns all error to the wrapper_function() function.
	 */
	void process_unbans();

	/**
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

public:
	using user_wrapper::user_wrapper;
};

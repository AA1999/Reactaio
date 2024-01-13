//
// Created by arshia on 1/13/24.
//

#pragma once

#include "../hybrid_wrapper.h"


/**
 * @brief hardban_wrapper - Wrapper class to hard ban a series of users and members.
 * @note Due to security reasons, this can only be called by the server owner.
 */
class hardban_wrapper: public hybrid_wrapper {

	/**
	 * @brief wrapper_function - The function that is called on operator() call and will process all the required steps to perform the operation.
	 */
	void wrapper_function() override;

	/**
	 * @brief check_permissions - Checks if the harban operation(s) can be performed (bot perms, author being owner etc)
	 */
	void check_permissions() override;

	/**
	 * @brief process_hardbans - Starts the hardban operation on all the given users/members.
	 */
	void process_hardbans();

	/**
	 * @brief process_response - Puts the wrapper result (errors + success) to the reply message.
	 */
	void process_response();

	bool invalid_user{false};

public:
	using hybrid_wrapper::hybrid_wrapper;
};

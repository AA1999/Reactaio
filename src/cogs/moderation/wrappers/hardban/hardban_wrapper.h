//
// Created by arshia on 1/13/24.
//

#pragma once

#include "../hybrid_wrapper.h"


/**
 * @brief hardban_wrapper - Wrapper class to hard ban a series of users and members.
 * @note Due to security reasons, this can only be called by the server owner.
 */
class hardban_wrapper final: public hybrid_wrapper {

	/**
	 * @brief wrapper_function - The function that is called on operator() call and will process all the required steps to perform the operation.
	 */
	void wrapper_function() override;

	/**
	 * @brief check_permissions - Checks if the harban operation(s) can be performed (bot perms, author being owner etc)
	 */
	void check_permissions() override;

	/**
	 * @brief process_softbans - Starts the hardban operation on all the given users/members.
	 */
	void process_hardbans();

	/**
	 * @brief process_response - Puts the wrapper result (errors + success) to the reply message.
	 */
	void process_response();

	/**
	 * @brief lambda_callback - This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param user User object that the callback is made on.
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, [[maybe_unused]] user_ptr const &user) override;

	bool invalid_user{false};

public:
	using hybrid_wrapper::hybrid_wrapper;

	~hardban_wrapper() override = default;
};

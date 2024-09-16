//
// Created by arshia on 1/14/24.
//

#pragma once

#include "../hybrid_wrapper.h"

/**
 * @brief A functor wrapper for handling softbans.
 */
class softban_wrapper: public hybrid_wrapper {
	/**
	 * @brief The function that is called on operator() call and will process all the required steps to perform the operation.
	 */
	void wrapper_function() override;

	/**
	 * @brief Checks if the softban operation(s) can be performed (bot perms, author being owner etc)
	 */
	void check_permissions() override;

	/**
	 * @brief Starts the softban operation on all the given users/members.
	 */
	void process_softbans();

	/**
	 * @brief Puts the wrapper result (errors + success) to the reply message.
	 */
	void process_response();

	/**
	 * @brief This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param user User object that the callback is made on.
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, [[maybe_unused]] user_ptr const &user) override;

	bool invalid_user{false};
public:
	using hybrid_wrapper::hybrid_wrapper;

	~softban_wrapper() override = default;
};

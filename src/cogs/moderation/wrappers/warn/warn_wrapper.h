//
// Created by arshia on 2/25/23.
//

#pragma once

#include "../member_wrapper.h"

/**
 * @brief warn_wrapper - Functor that processes the warn operation for a list of dpp::guild_member objects.
 */
class warn_wrapper final: public member_wrapper {
	/**
	 * @brief wrapper_function - Function called when the () operator is called. Warns all the given members in the guild and returns any errors/success message.
	 */
	void wrapper_function() override;

	/**
	 * @brief check_permissions - Checks if the command invoker has the permission to warn anyone from the guild (including the bot).
	 * @note This function works internally and returns all error to the wrapper_function() function.
	 */
	void check_permissions() override;

	/**
	 * @brief process_bans - Bans the list of users/members from the guild and returns errors/success message.
	 * @note This function works internally and returns all error to the wrapper_function() function.
	 */
	void process_warnings();

	/**
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

	/**
	 * @brief lambda_callback - This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param member Member object that the callback is made on.
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, [[maybe_unused]] member_ptr const &member) override;
public:
	using member_wrapper::member_wrapper;
};

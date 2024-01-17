//
// Created by arshia on 1/17/24.
//

#pragma once

#include "../member_wrapper.h"

class unmute_wrapper: public member_wrapper {
	/**
	 * @brief wrapper_function - Function called when the () operator is called. Mutes/Times out all the given members in the guild and returns any errors/success message.
	 */
	void wrapper_function() override;

	/**
	 * @brief check_permissions - Checks if the command invoker has the appropriate permissions to perform the operation (including the bot itself).
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void check_permissions() override;

	/**
	 * @brief process_unmutes - Unmutes the list of dpp::guild_member. Shows all errors or success message.
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void process_unmutes();

	/**
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

	bool use_timeout{true};

	/**
	 * @brief lambda_callback - This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param member Member object that the callback is made on.
	 */
	void lambda_callback(dpp::confirmation_callback_t const& completion, [[maybe_unused]] dpp::guild_member const& member) override;

	bool mute_callback{false};

public:
	using member_wrapper::member_wrapper;

	~unmute_wrapper() override = default;
};

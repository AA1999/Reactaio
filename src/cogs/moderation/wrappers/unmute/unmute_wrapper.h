//
// Created by arshia on 1/17/24.
//

#pragma once

#include "../member_wrapper.h"

/**
 * @brief A functor wrapper for handling member unmutes,
 */
class unmute_wrapper final: public member_wrapper {
	/**
	 * @brief Function called when the () operator is called. Mutes/Times out all the given members in the guild and returns any errors/success message.
	 */
	void wrapper_function() override;

	/**
	 * @brief Checks if the command invoker has the appropriate permissions to perform the operation (including the bot itself).
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void check_permissions() override;

	/**
	 * @brief Unmutes the list of dpp::guild_member. Shows all errors or success message.
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void process_unmutes();

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();


	/**
	 * @brief This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param member Member object that the callback is made on.
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, [[maybe_unused]] member_ptr const &member) override;

	bool use_mute_callback{false};

public:
	using member_wrapper::member_wrapper;

	~unmute_wrapper() override = default;
};

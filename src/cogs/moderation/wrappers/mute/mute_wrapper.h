//
// Created by arshia on 3/4/23.
//

#pragma once

#include "../member_wrapper.h"


/**
 * @brief Functor that processes the mute/timeout operation on a list of dpp::guild_member objects.
 */
class mute_wrapper final: public member_wrapper {
	/**
	 * @brief Function called when the () operator is called. Mutes/Times out all the given members in the guild and returns any errors/success message.
	 */
	void wrapper_function() override;

	/**
	 * @brief Checks if all the users can be muted.
	 * @param protected_roles Roles protected from moderation actions.
	 */
	void check_mute_possible(const shared_vector<dpp::role>& protected_roles);

	/**
	 * @brief Checks if the command invoker has the appropriate permissions to perform the operation (including the bot itself).
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void check_permissions() override;

	/**
	 * @brief Mutes the list of dpp::guild_member. Shows all errors or success message.
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void process_mutes();

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

	bool use_timeout{true};

	/**
	 * @brief This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param member Member object that the callback is made on.
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, [[maybe_unused]] member_ptr const &member) override;

public:
	using member_wrapper::member_wrapper;

	~mute_wrapper() override = default;
};

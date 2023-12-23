//
// Created by arshia on 2/15/23.
//

#pragma once
#include "../member_wrapper.h"

/**
 * @brief kick_wrapper - Functor that processes the kick operation on a list of dpp::guild_member objects.
 */
class kick_wrapper: public member_wrapper {
	/**
	 * @brief wrapper_function - Function called when the () operator is called. Kicks all the given members from the guild and returns any errors/success message.
	 */
	void wrapper_function() override;

	/**
	 * @brief check_permissions - Checks if the command invoker has the appropriate permissions to perform the operation (including the bot itself).
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void check_permissions() override;

	/**
	 * @brief process_kicks - Kicks the list of dpp::guild_member from the guild. Shows all errors or success message.
	 * @note This is called internally and will return the result via the wrapper_function() function.
	 */
	void process_kicks();

	/**
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

  public:
	using member_wrapper::member_wrapper;
};

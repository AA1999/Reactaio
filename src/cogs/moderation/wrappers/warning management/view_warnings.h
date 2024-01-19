//
// Created by arshia on 1/19/24.
//

#pragma once

#include <utility>

#include "../information_wrapper.h"

/**
 * @brief view_warnings - Wrapper for viewing warnings for a member.
 */
class view_warnings: public information_wrapper {

	dpp::guild_member member;

	dpp::message response;

	/**
	 * 	@brief check_permissions - Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * 	@brief wrapper_function - The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief find_warnings -
	 */
	void find_warnings();

	/**
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

public:
	view_warnings(moderation_command command, dpp::guild_member member):information_wrapper(std::move(command)), member(std::move(member)){}
	~view_warnings() override = default;

};

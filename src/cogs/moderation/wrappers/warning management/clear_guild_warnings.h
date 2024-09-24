//
// Created by arshia on 1/20/24.
//

#pragma once

#include "../simple_wrapper.h"

/**
 * @brief Wrapper to handle clearing guild warnings.
 */
class clear_guild_warnings final: public simple_wrapper {
	dpp::message response;

	/**
	 * 	@briefChecks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * 	@brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

public:
	using simple_wrapper::simple_wrapper;
	~clear_guild_warnings() override = default;
};

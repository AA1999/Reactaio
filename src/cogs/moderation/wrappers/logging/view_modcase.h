//
// Created by arshia on 1/22/24.
//

#pragma once

#include <utility>

#include "../simple_wrapper.h"


class view_modcase final: public simple_wrapper {
	std::string case_id;
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
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

public:
	/**
	 * @brief The main constructor of the class used to get data from the command.
	 * @param case_id The case id to view.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	view_modcase(std::string case_id, moderation_command command): simple_wrapper(std::move(command)), case_id(std::move(case_id)){}
	~view_modcase() override = default;

};

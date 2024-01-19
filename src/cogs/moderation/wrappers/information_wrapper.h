//
// Created by arshia on 1/18/24.
//

#pragma once

#include "command_wrapper.h"


class information_wrapper: public command_wrapper {

	/**
	 * 	@brief check_permissions - Checks if the user issuing the wrapper has the sufficient permission.
	 * 	@note This is an abstract function.
	 */
	void check_permissions() override = 0;

	/**
	 * 	@brief wrapper_function - The main function that manages every internal working of the wrapper and the three processes that are performed.
	 * 	@note This is an abstract function.
	 */
	void wrapper_function() override = 0;
public:

	information_wrapper(moderation_command command): command_wrapper(std::move(command)){}

	~information_wrapper() override = default;

	[[nodiscard]] bool are_all_errors() const override;
};

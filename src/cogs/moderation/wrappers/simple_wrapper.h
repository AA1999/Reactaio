//
// Created by arshia on 1/18/24.
//

#pragma once

#include "command_wrapper.h"

/**
 * @brief simple_wrapper - A wrapper used to perform commands which do not need an API call and are only structured similarly.
 */
class simple_wrapper: public command_wrapper {

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
	/**
	 @brief The main constructor of the class used to get data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit simple_wrapper(moderation_command command): command_wrapper(std::move(command)){}

	~simple_wrapper() override = default;
	simple_wrapper() = delete;

	[[nodiscard]] constexpr bool are_all_errors() const override {
		return has_error();
	}
};

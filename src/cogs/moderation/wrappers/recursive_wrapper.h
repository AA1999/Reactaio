//
// Created by arshia on 3/6/24.
//

#pragma once

#include "command_wrapper.h"

class recursive_wrapper: public command_wrapper{
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

	/**
	*@brief recursive_call - Recursive call needed to get all the data.
	*/
	virtual void recursive_call(dpp::snowflake after = 1) = 0;
public:
	/**
	 @brief The main constructor of the class used to get data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit recursive_wrapper(moderation_command command): command_wrapper(std::move(command)){}

	~recursive_wrapper() override = default;
	recursive_wrapper() = delete;

	recursive_wrapper(const recursive_wrapper& other) = delete;
	recursive_wrapper(recursive_wrapper&& other) = delete;

	[[nodiscard]] constexpr bool are_all_errors() const override {
		return has_error();
	}
};

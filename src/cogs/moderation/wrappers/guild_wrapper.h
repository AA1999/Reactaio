//
// Created by arshia on 1/15/24.
//

#pragma once

#include "command_wrapper.h"

/**
 * @brief guild_wrapper - Wrapper used to handle operations on a specific guild.
 */
class guild_wrapper: public command_wrapper {

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
	 * @brief lambda_callback - This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param user User object that the callback is made on.
	 */
	virtual void lambda_callback(dpp::confirmation_callback_t const& completion, dpp::user* user) = 0;

public:
	guild_wrapper() = delete;
	~guild_wrapper() override = default;

	/**
	 * @brief The main constructor of the class used to get data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit guild_wrapper(moderation_command& command): command_wrapper(std::move(command)){};

	/**
	 * @brief are_all_errors - Checks if the operation had any errors.
	 * @return true if there were errors in the operation.
	 * @return false if there were no errors in the operation.
	 */
	[[nodiscard]] constexpr bool are_all_errors() const override {
		return has_error();
	}
};

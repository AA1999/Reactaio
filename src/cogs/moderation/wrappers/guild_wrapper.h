//
// Created by arshia on 1/15/24.
//

#pragma once

#include "command_wrapper.h"

/**
 * @brief Wrapper used to handle operations on a specific guild.
 */
class guild_wrapper: public command_wrapper {

	/**
	 * 	@brief Checks if the user issuing the wrapper has the sufficient permission.
	 * 	@note This is an abstract function.
	 */
	void check_permissions() override = 0;

	/**
	 * 	@brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 * 	@note This is an abstract function.
	 */
	void wrapper_function() override = 0;

	/**
	 * @brief This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 */
	virtual void lambda_callback(dpp::confirmation_callback_t const& completion) = 0;

	/**
	 * @brief Logs the modcase action.
	 * @param command_name Command name to add to the modlog.
	 */
	void log_modcase(std::string_view const& command_name) const override;

public:
	guild_wrapper() = delete;
	~guild_wrapper() override = default;

	guild_wrapper(const guild_wrapper& other) = delete;
	guild_wrapper(guild_wrapper&& other) = delete;

	/**
	 * @brief The main constructor of the class used to get data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit guild_wrapper(moderation_command command): command_wrapper(std::move(command)){};

	/**
	 * @brief Checks if the operation had any errors.
	 * @return true if there were errors in the operation.
	 * @return false if there were no errors in the operation.
	 */
	[[nodiscard]] constexpr bool are_all_errors() const override {
		return has_error();
	}
};

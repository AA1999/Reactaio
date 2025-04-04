//
// Created by arshia on 3/12/23.
//

#pragma once

#include "command_wrapper.h"


#include <variant>

/**
 * @brief A wrapper used to process commands with a mix of dpp::user* and dpp::guild_member elements.
 */

class hybrid_wrapper: public command_wrapper {
protected:

	internal::unique_vector<member_user_variant> snowflakes;
	shared_vector<dpp::user> users_with_errors;
	shared_vector<dpp::user> users;
	shared_vector<dpp::guild_member> members;

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
	 * @param completion On success, the callback will contain a dpp::confirmation object in confirmation_callback_t::value.
	 * On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param user User object that the callback is made on.
	 */
	virtual void lambda_callback(dpp::confirmation_callback_t const& completion, user_ptr const& user) = 0;

	/**
	 * @brief Logs the moderation action in the modcase database.
	 * @param command_name The command name to log in the modcase.
	 */
	void log_modcase(std::string_view const& command_name) const;

public:

	hybrid_wrapper() = delete;
	~hybrid_wrapper() override = default;


	hybrid_wrapper(const hybrid_wrapper& other) = delete;
	hybrid_wrapper(hybrid_wrapper&& other) = delete;

	/**
	 * @brief Checks if every item has encountered an error.
	 * @return Whether if every single item given to the wrapper has encountered an error.
	 */
	[[nodiscard]] constexpr bool are_all_errors() const override {
		return users_with_errors.size() == snowflakes.size();
	}

	/**
	 * @brief The main constructor of the class used to get data from the command.
	 * @param snowflakes The list of dpp::user* and dpp::guild_member objects.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	hybrid_wrapper(internal::unique_vector<member_user_variant> const& snowflakes, moderation_command& command): command_wrapper(std::move(command)), snowflakes(snowflakes) {}
};

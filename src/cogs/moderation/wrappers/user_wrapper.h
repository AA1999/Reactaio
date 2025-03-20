//
// Created by arshia on 3/12/23.
//

#pragma once

#include "command_wrapper.h"

/**
 * @brief A wrapper used to process commands of dpp::user* elements.
 */
class user_wrapper: public command_wrapper {
protected:
	shared_vector<dpp::user> users;
	shared_vector<dpp::user> users_with_errors;

	/**
	 * @brief check_permissions - Checks if the user issuing the wrapper has the sufficient permission.
	 * @note This is an abstract function.
	 */
	void check_permissions() override = 0;

	/**
	 * @brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 * @note This is an abstract function.
	 */
	void wrapper_function() override = 0;

	/**
	 * @brief Logs the moderation action in the database.
	 * @param command_name The name of the command to log the mod case for.
	 */
	void log_modcase(std::string_view const& command_name) const;

	/**
	 * @brief This is a function called when an API call is made.
	 * @param completion On success, the callback will contain a dpp::confirmation object in confirmation_callback_t::value.
	 * On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param user User object that the callback is made on.
	 */
	virtual void lambda_callback(dpp::confirmation_callback_t const &completion, [[maybe_unused]] user_ptr const &user) = 0;

public:
	/**
	 * @brief The constructor used to receive the data from the command.
	 * @param users The list of dpp::user* objects.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	user_wrapper(const shared_vector<dpp::user>& users, moderation_command& command): command_wrapper(std::move(command)), users(users){}

	/**
	 * @brief Checks if every item has encountered an error.
	 * @return Whether if every single item given to the wrapper has encountered an error.
	 */
	[[nodiscard]] constexpr bool are_all_errors() const override {
		return users_with_errors.size() == users.size();
	}


	~user_wrapper() override = default;
	user_wrapper() = delete;

	user_wrapper(user_wrapper const& other) = delete;
	user_wrapper(user_wrapper&& other) = delete;
};

//
// Created by arshia on 2/16/23.
//

#pragma once

#include "../../base/datatypes/duration.h"
#include "../../base/helpers.h"
#include "../modules/moderation_command.h"

#include <dpp/dpp.h>
#include <optional>
#include <pqxx/pqxx>
#include <string_view>
#include <utility>
#include <vector>

/**
 * \brief base_wrapper - The base abstract functor class for all moderation actions. This class is only supposed to be used as a base class and not have direct instances
 */

class base_wrapper {

protected:
	std::optional<duration> duration;

	std::vector<std::string> errors{};
	std::vector<dpp::embed> embeds;

	moderation_command command;

	dpp::message error_message;

	bool cancel_operation{false};

	/**
	 * 	@brief check_permissions - Checks if the user issuing the wrapper has the sufficient permission.
	 * 	@note This is an abstract function.
	 */
	virtual void check_permissions() = 0;

	/**
	 * 	@brief wrapper_function - The main function that manages every internal working of the wrapper and the three processes that are performed.
	 * 	@note This is an abstract function.
	 */
	virtual void wrapper_function() = 0;


public:
	virtual ~base_wrapper() = 0;

	base_wrapper() = delete;

	/**
	 * @brief The constructor for the wrapper.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit base_wrapper(moderation_command command)
	    : command(std::move(command)), duration(parse_human_time(command.duration)) {}


	/**
	 * @brief operator() - Called when the functor is invoked.
	 */
	virtual void operator()() final;

	/**
	 * @brief has_error - Checks for any errors in the process.
	 * @return Whether there is any error when performing the specified instructions.
	 */
	[[nodiscard]] virtual bool has_error() const final;

	/**
	 * @brief are_all_errors - Checks if every item has encountered an error.
	 * @return Whether if every single item given to the wrapper has encountered an error.
	 */
	[[nodiscard]] virtual bool are_all_errors() const = 0;

	/**
	 * @brief error - Gives the error message of the wrapper result.
	 * @note This is an abstract function.
	 * @return The full error message if has_error() is true
	 */
	[[nodiscard]] std::optional<dpp::message> error() const;
};

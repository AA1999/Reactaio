//
// Created by arshia on 2/16/23.
//

#pragma once


#include "../../core/discord/duration.h"
#include "../../core/helpers.h"
#include "../modules/moderation_command.h"

#include <dpp/dpp.h>
#include <optional>
#include <utility>
#include <vector>

namespace internal = reactaio::internal;

/**
 * @brief The base abstract functor class for all moderation actions. This class is only supposed to be used as a base class and not have direct instances
 */
class command_wrapper {

protected:

	std::optional<internal::duration> duration;

	std::vector<std::string> errors{};
	std::vector<dpp::embed> embeds;

	moderation_command command;

	dpp::message error_message;

	bool cancel_operation{false};

	/**
	 * 	@brief Checks if the user issuing the wrapper has the sufficient permission.
	 * 	@note This is an abstract method.
	 */
	virtual void check_permissions() = 0;

	/**
	 * 	@brief The main method that manages every internal working of the wrapper and the three processes that are performed.
	 * 	@note This is an abstract method.
	 */
	virtual void wrapper_function() = 0;

	/**
	 * @brief Logs the command being invoked.
	 * @param name Name of the command. Must be one of mod_action.h strings.
	 */
	void log_command_invoke(std::string_view const& name) const;

	/**
	 * @brief Call the error webhook and send the error there.
	 */
	void invoke_error_webhook();

	/**
	 * @brief Gets the protected role for the guild that the command is called in.
	 */
	void get_protected_roles() const;

	/**
	 * @brief Gets the roles that are permitted to run this command.
	 * @param command_name The command name that the mod perms are being retrieved for.
	 */
	void get_permitted_roles(std::string_view const& command_name) const;

	/**
	 * @brief Logs the action inside the modcase.
	 * @param command_name The command name that the modcase is being invoked for.
	 * @note This is an abstract method.
	 */
	virtual void log_modcase(std::string_view const& command_name) const = 0;

public:
	virtual ~command_wrapper() = default;

	command_wrapper() = delete;

	/**
	 * @brief The constructor for the wrapper.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit command_wrapper(moderation_command command) : duration(parse_human_time(command.duration)), command(std::move(command)) {
		replace_all(command.reason, "@everyone", "");
		replace_all(command.reason, "@here", "");
	}


	/**
	 * @brief Called when the functor is invoked.
	 */
	void operator()();

	/**
	 * @brief Checks for any errors in the process.
	 * @return Whether there is any error when performing the specified instructions.
	 */
	[[nodiscard]] constexpr bool has_error() const {
		return !errors.empty();
	}

	/**
	 * @brief Checks if every item has encountered an error.
	 * @return true if every item in the wrapper has encountered an error.
	 * @return false if at least one item in the wrapper had finished with no errors.
	 * @note This is an abstract function.
	 */
	[[nodiscard]] virtual bool are_all_errors() const = 0;

	/**
	 * @brief Gives the error message of the wrapper result.
	 * @note This is an abstract function.
	 * @return The full error message if has_error() is true.
	 */
	[[nodiscard]]std::optional<dpp::message> error() const;

	command_wrapper(const command_wrapper& other) = delete;
	command_wrapper(command_wrapper&& other) = delete;
};

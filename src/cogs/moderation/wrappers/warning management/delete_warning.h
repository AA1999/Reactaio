//
// Created by arshia on 1/20/24.
//

#pragma once

#include "../simple_wrapper.h"

/**
 * @brief Wrapper to handle deleting a warning.
 */
class delete_warning final: public simple_wrapper {
	std::string warning_id;

	dpp::guild_member member;
	dpp::message response;

	/**
	 * 	@brief Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * 	@brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();
public:
	delete_warning() = delete;

	/**
	 @brief The main constructor of the class used to get data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 * @param warning_id The warning id.
	 */
	delete_warning(std::string_view const warning_id, moderation_command command) : simple_wrapper(std::move(command)), warning_id(warning_id){}

	~delete_warning() override = default;
};

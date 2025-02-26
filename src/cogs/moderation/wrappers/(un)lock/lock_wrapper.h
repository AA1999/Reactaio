//
// Created by arshia on 3/18/24.
//

#pragma once 

#include "../channel_wrapper.h"

/**
 * @brief Wrapper to handle locking a channel in the server.
 */
class lock_wrapper final: public channel_wrapper {
	/**
	 * @brief Checks if both the command invoker and the bot have sufficient permissions.
	 */
	void check_permissions() override;

	/**
	 * @brief Function called when the () operator is called. Locks all the given channels.
	 */
	void wrapper_function() override;

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response() override;

	/**
	 * @brief Lambda callback after the API calls are made.
	 * @param completion The compeltion object that indicates if the API call was suceessful.
	 * @param channel Channel that the API call is made for.
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, channel_ptr const &channel) override;

	/**
	 * @brief Logs a modcase of the action happening.
	 * @param command_name Command name to log the modcase for.
	 */
	void log_modcase(std::string_view const &command_name) const override;

	/**
	 * @brief Locks all the given channels. On error the errors will be sent to the errors vector.
	 */
	void process_locks();
public:
	lock_wrapper() = delete;
	~lock_wrapper() override = default;
	using channel_wrapper::channel_wrapper;
};

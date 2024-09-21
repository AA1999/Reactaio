//
// Created by arshia on 3/18/24.
//

#pragma once
#include <utility>

#include "../channel_wrapper.h"

/**
 * @brief A wrapper for server lockdown.
 */
class lockdown_wrapper final: public channel_wrapper {

	/**
	 * @brief Checks if both the command invoker and the bot have sufficient permissions.
	 */
	void check_permissions() override;

	/**
	 * @brief Function called when the () operator is called. Unlocks all the given channels.
	 */
	void wrapper_function() override;

	/**
	 * @brief Lambda callback after the API calls are made.
	 * @param completion The compeltion object that indicates if the API call was suceessful.
	 * @param channel Channel that the API call is made for.
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, channel_ptr const &channel) override;

	/**
	 * @brief Locks all the set server channels. On error the errors will be sent to the errors vector.
	 */
	void process_lockdown();

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response() override;

public:
	lockdown_wrapper() = delete;
	~lockdown_wrapper() override = default;

	/**
	 * @brief Constructor from a command object.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	explicit lockdown_wrapper(moderation_command command): channel_wrapper(std::move(command), {}) {}
};

//
// Created by arshia on 3/18/24.
//

#pragma once
#include <utility>

#include "../channel_wrapper.h"


class lockdown_wrapper: public channel_wrapper {

	shared_vector<dpp::channel> channel_ptrs;

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
	void lambda_callback(dpp::confirmation_callback_t const &completion, dpp::channel const &channel) override;

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
	explicit lockdown_wrapper(moderation_command command): channel_wrapper(std::move(command), {}) {}

	/**
	 * @brief are_all_errors - Checks if the operation had any errors.
	 * @return true if there were errors in the operation.
	 * @return false if there were no errors in the operation.
	 */
	[[nodiscard]] constexpr bool are_all_errors() const override {
		return channels_with_errors.size() == channel_ptrs.size();
	}

};

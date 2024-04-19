//
// Created by arshia on 3/18/24.
//

#pragma once
#include "../channel_wrapper.h"


class unlock_wrapper final: public channel_wrapper {
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
	 * @brief Unlocks all the given channels. On error the errors will be sent to the errors vector.
	 */
	void process_unlocks();

	void process_response() override;

public:
	unlock_wrapper() = delete;
	~unlock_wrapper() override = default;
	using channel_wrapper::channel_wrapper;
};

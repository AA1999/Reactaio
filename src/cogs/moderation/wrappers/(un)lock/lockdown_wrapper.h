//
// Created by arshia on 3/18/24.
//

#pragma once
#include "../guild_wrapper.h"


class lockdown_wrapper: public guild_wrapper {
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

public:
	lockdown_wrapper() = delete;
	~lockdown_wrapper() override = default;
	using guild_wrapper::guild_wrapper;
};

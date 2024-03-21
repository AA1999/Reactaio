//
// Created by arshia on 3/17/24.
//

#pragma once

#include "command_wrapper.h"

class channel_wrapper: public command_wrapper {
protected:
	std::vector<dpp::channel> channels;
	shared_vector<dpp::channel> channels_with_errors;
	/**
	 * 	@brief check_permissions - Checks if the user issuing the wrapper has the sufficient permission.
	 * 	@note This is an abstract function.
	 */
	void check_permissions() override = 0;

	/**
	 * 	@brief wrapper_function - The main function that manages every internal working of the wrapper and the three processes that are performed.
	 * 	@note This is an abstract function.
	 */
	void wrapper_function() override = 0;

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	virtual void process_response() = 0;

	/**
	 * @brief lambda_callback - This is a function that's called when an API call is made.
	 * @param completion On success the callback will contain a dpp::confirmation object in confirmation_callback_t::value. On failure, the value is undefined and confirmation_callback_t::is_error() method will return true.
	 * @param channel User object that the callback is made on.
	 */
	virtual void lambda_callback(dpp::confirmation_callback_t const &completion, dpp::channel const &channel) = 0;

	/**
	 * @brief Inserts all the channel objects with no error into the given vector.
	 * @param output The vector to put the result in. This way we can have a constexpr function.
	 */
	constexpr void filter(shared_vector<dpp::channel>& output) {
		std::ranges::copy(channels | std::views::filter([this](dpp::channel const& channel){return !contains(channels_with_errors, std::make_shared<dpp::channel>(channel));})
								  | std::views::transform([](dpp::channel const& channel){return std::make_shared<dpp::channel>(channel);}),
									std::back_inserter(output));
	}

public:
	channel_wrapper() = delete;
	~channel_wrapper() override = default;

	/**
	 * @brief The main constructor of the class used to get data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 * @param channels The channels that the command is used for.
	*/
	explicit channel_wrapper(moderation_command command, const std::vector<dpp::channel>& channels): command_wrapper(std::move(command)), channels(channels){};

	/**
	 * @brief are_all_errors - Checks if the operation had any errors.
	 * @return true if there were errors in the operation.
	 * @return false if there were no errors in the operation.
	 */
	[[nodiscard]] constexpr bool are_all_errors() const override {
		return channels_with_errors.size() == channels.size();
	}
};
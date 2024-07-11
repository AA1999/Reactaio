//
// Created by arshia on 5/21/24.
//

#pragma once
#include "../simple_wrapper.h"
#include "../../mod_action.h"


class change_duration final: simple_wrapper {
	std::string case_id;
	dpp::message response;
	internal::duration new_duration;

	static std::array constexpr actions_with_duration = {internal::mod_action_name::BAN, internal::mod_action_name::MUTE, internal::mod_action_name::TIMEOUT};

	/**
	 * 	@brief Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * 	@brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief Updates the duration for the given user_id.
	 */
	void update_duration();

	/**
	 * @brief process_response - Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();
public:
	change_duration() = delete;
	~change_duration() override = default;

	/**
	 @brief The main constructor of the class used to get data from the command.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 * @param case_id The case to change the duration for.
	 * @param new_duration The new duration for the command.
	 */
	change_duration(moderation_command command, std::string case_id, internal::duration const& new_duration): simple_wrapper(std::move(command)), case_id(std::move(case_id)), new_duration(new_duration){}
};

//
// Created by arshia on 9/12/24.
//

#pragma once
#include "../simple_wrapper.h"

/**
 * @brief Wrapper functor to view all modcases regarding a user.
 */
class view_user_modcases: public simple_wrapper {
	member_ptr m_member;
public:
	view_user_modcases() = delete;
	~view_user_modcases() override = default;

	/**
	 * @brief Creates an instance based on the given member and the command metadata.
	 * @param member Member to lookup all the modlogs for.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	view_user_modcases(member_ptr member, moderation_command command): m_member(std::move(member)), simple_wrapper(std::move(command)) {};

	/**
	 * @brief Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * @brief Shows all the modcases for the user.
	 */
	void process_response();

	/**
	 * @brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;
};

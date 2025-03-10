//
// Created by arshia on 9/21/24.
//

#pragma once
#include "../simple_wrapper.h"

/**
 * @brief Functor wrapper to view all modcases issued by a moderator.
 */
class view_mod_modcases final: public simple_wrapper {
	dpp::message response;
	member_ptr mod;
public:
	/**
	 * 	@brief Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * @brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief Shows all the modcases for the mod.
	 */
	void process_response();

	/**
	 * @brief The main constructor of the class used to get data from the command.
	 * @param mod Moderator/Admit to look up the modcases for.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	explicit view_mod_modcases(member_ptr mod, moderation_command command): simple_wrapper(std::move(command)), mod(std::move(mod)) {};

};

//
// Created by arshia on 2/24/23.
//

#pragma once

#include "../hybrid_wrapper.h"

/**
 * @brief Functor that processes the ban operation for a list of dpp::guild_member or dpp::user* objects.
 * @note This is only for the normal ban operation. Softban/Hardban operations have a different wrapper.
 */
class ban_wrapper final: public hybrid_wrapper {
	/**
	 * @brief Function called when the () operator is called. Bans all the given members/users from the guild and returns any errors/success message.
	 */
	void wrapper_function() override;

	/**
	 * @brief Checks if the command invoker has the permission to ban anyone from the guild (including the bot).
	 * @note This function works internally and returns all error to the wrapper_function() function.
	 */
	void check_permissions() override;

	/**
	 * @brief Bans the list of users/members from the guild and returns errors/success message.
	 * @note This function works internally and returns all error to the wrapper_function() function.
	 */
	void process_bans();

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

	/**
	 * @brief Check if all members in the member list are permitted to be banned.
	 * @param protected_roles Protected roles that are immune to moderation commands. Obtained in check_permissions()
	 */
	void check_ban_possible(shared_vector<dpp::role> const& protected_roles);

	/**
	 * @brief This is a function that's called when an API call is made.
	 * @param completion
	 * @param user The user object the comeback is
	 */
	void lambda_callback(dpp::confirmation_callback_t const &completion, [[maybe_unused]] user_ptr const &user) override;

	/**
	 * @brief Whether there was an invalid user found in the command users.
	 */
	bool invalid_user{false};
public:
	using hybrid_wrapper::hybrid_wrapper;

	~ban_wrapper() override = default;
};

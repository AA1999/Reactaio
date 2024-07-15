//
// Created by arshia on 2/15/23.
//

#pragma once

#include "../../core/aliases.h"
#include "moderation_command.h"

#include <dpp/dpp.h>
#include <variant>
#include <vector>

namespace reactaio::moderation {

	/**
	 * @brief Removes a member/list of members from the guild.
	 * @param members The members that are supposed to be removed.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for kick, server settings etc.
	 */
	void kick(const shared_vector<dpp::guild_member> &members, moderation_command command);

	/**
	 * @brief Bans a member/user/list of members and users from the guild.
	 * @param users_or_members The mix of members and users that are supposed to be removed.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for kick, server settings etc.
	 */
	void ban(const internal::unique_vector<member_user_variant> &users_or_members, moderation_command command);

	/**
	 * @brief Bans and then unbans a member/user/list of members and users from the guild. (Used to purge messages from former/current members).
	 * @param users_or_members The mix of members and users that are supposed to be removed.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for ban, server settings etc.
	 */
	void softban(const internal::unique_vector<member_user_variant> &users_or_members, moderation_command command);

	/**
	 * @brief Bans a member permanently. Only the server owner can use this command and unban hardbanned members/users.
	 * @param users_or_members The mix of members and users that are supposed to be hardbanned.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for ban, server settings etc.
	 */
	void hardban(const internal::unique_vector<member_user_variant> &users_or_members, const moderation_command &command);

	/**
	 * @brief Removes the ability for a member/list of members to chat. Uses the discord timeout feature if specified.
	 * @param members The list of members to mute.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for mute, server settings etc.
	 */
	void mute(const shared_vector<dpp::guild_member> &members, moderation_command command);

	/**
	 * @brief Warns a member for actions that are caused.
	 * @param members The list of members to warn.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for warn, server settings etc.
	 */
	void warn(const shared_vector<dpp::guild_member> &members, moderation_command command);

	/**
	 * @brief Deletes a warning by id.
	 * @param warning_id The warning id that's unique to each guild.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	void delete_warn(warn_t warning_id, moderation_command command);

	/**
	 * @brief Removes all warnings for a guild member.
	 * @param member Member to clear all warns for.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	void clear_member_warnings(member_ptr member, moderation_command command);

	/**
	 * @brief Deletes all the warnings from a server. Can only be used by the server owner for security reasons.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	void delete_all_guild_warns(moderation_command command);

	/**
	 * @brief Removes the mute punishment from a member/list of members.
	 * @param members - The list of members that are muted
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for unmute, server settings etc.
	 */
	void unmute(const shared_vector<dpp::guild_member> &members, moderation_command command);

	/**
	 * @brief Unbans the users from the server.
	 * @param users List of banned users.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for unban, server settings etc.
	 * @note If a user is hard banned, only the owner would be able to unban that user.
	 */
	void unban(const shared_vector<dpp::user> &users, moderation_command command);

	/**
	 * @brief View all the bans of the server as a paginated message.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	void view_ban_list(moderation_command command);

	/**
	 * @brief View all currently muted/timed out members of the server.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	void view_muted_list(moderation_command command);

	/**
	 * @brief View a moderation case.
	 * @param case_id
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	void view_modase(case_t case_id, moderation_command command);

	/**
	 * @brief View all warnings for a member of the server.
	 * @param member The member for the warning lookup.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for the action, server settings etc.
	 */
	void view_member_warnings(member_ptr member, moderation_command command);

	/**
	 * @brief View all warnings for the server members as a paginated messsage.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for kick, server settings etc.
	 */
	void view_guild_warnings(moderation_command command);

	/**
	 * @brief Updates the reason for a modcase.
	 * @param case_id The case id to update reason for.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for kick, server settings etc.
	 */
	void update_reason(case_t case_id, moderation_command command);

	/**
	 * @brief Updates the duration for a modcase. However this won't work for a command without duration.
	 * @param case_id The case id to update duration for.
	 * @param command The wrapper struct containing all the information for the action. i.e. guild, reason for kick, server settings etc.
	 */
	void update_duration(case_t case_id, moderation_command command);

}
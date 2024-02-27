//
// Created by arshia on 2/15/23.
//

#pragma once

#include "moderation_command.h"

#include <dpp/dpp.h>
#include <variant>
#include <vector>

namespace reactaio::moderation {
	/**
	 * @brief kick - Removes a member/list of members from the guild.
	 * @param members The members that are supposed to be removed.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for kick, server settings etc.
	 */
	void kick(const std::vector<dpp::guild_member>& members, moderation_command command);

	/**
	 * @brief ban - Bans a member/user/list of members and users from the guild.
	 * @param users_or_members The mix of members and users that are supposed to be removed.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for kick, server settings etc.
	 */
	void ban(const std::vector<std::variant<dpp::guild_member, dpp::user*>>& users_or_members, moderation_command command);

	/**
	 * @brief softban - Bans and then unbans a member/user/list of members and users from the guild. (Used to purge messages from former/current members).
	 * @param users_or_members The mix of members and users that are supposed to be removed.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for ban, server settings etc.
	 */
	void softban(const std::vector<std::variant<dpp::guild_member, dpp::user*>>& users_or_members, moderation_command command);

	/**
	 * @brief hardban - Bans a member permanently. Only the server owner can use this command and unban hardbanned members/users.
	 * @param users_or_members The mix of members and users that are supposed to be hardbanned.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for ban, server settings etc.
	 */
	void hardban(const std::vector<std::variant<dpp::guild_member, dpp::user*>>& users_or_members, const moderation_command& command);

	/**
	 * @brief mute - Removes the ability for a member/list of members to chat. Uses the discord timeout feature if specified.
	 * @param members The list of members to mute.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for mute, server settings etc.
	 */
	void mute(const std::vector<dpp::guild_member>& members, moderation_command command);

	/**
	 * @brief warn - Warns a member for actions that are caused.
	 * @param members The list of members to warn.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for warn, server settings etc.
	 */
	void warn(const std::vector<dpp::guild_member>& members, moderation_command command);

	/**
	 * @brief delete_warn - Deletes a warning by id.
	 * @param warning_id The warning id that's unique to each guild.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for the action, server settings etc.
	 */
	void delete_warn(warn_t warning_id, moderation_command command);

	/**
	 * @brief clear_member_warnings - Removes all warnings for a guild member.
	 * @param member Member to clear all warns for.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for the action, server settings etc.
	 */
	void clear_member_warnings(dpp::guild_member member, moderation_command command);

	/**
	 * @brief delete_all_guild_warns - Deletes all the warnings from a server. Can only be used by the server owner for security reasons.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for the action, server settings etc.
	 */
	void delete_all_guild_warns(moderation_command command);

	/**
	 * @brief unmute - Removes the mute punishment from a member/list of members.
	 * @param members - The list of members that are muted
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for unmute, server settings etc.
	 */
	void unmute(const std::vector<dpp::guild_member>& members, moderation_command command);

	/**
	 * @brief unban - Unbans the users from the server.
	 * @param users List of banned users.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for unban, server settings etc.
	 * @note If a user is hard banned, only the owner would be able to unban that user.
	 */
	void unban(const std::vector<dpp::user*>& users, moderation_command command);

	/**
	 * @brief view_ban_list - View all the bans of the server as a paginated message.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for the action, server settings etc.
	 */
	void view_ban_list(moderation_command command);

	/**
	 * @brief view_muted_list - View all currently muted/timed out members of the server.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for the action, server settings etc.
	 */
	void view_muted_list(moderation_command command);

	/**
	 * @brief view_member_warnings - View all warnings for a member of the server.
	 * @param member The member for the warning lookup.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for the action, server settings etc.
	 */
	void view_member_warnings(dpp::guild_member member, moderation_command command);

	/**
	 * @brief view_guild_warnings - View all warnings for the server members as a paginated messsage.
	 * @param command The wrapper struct containing all the information for the action. ie. guild, reason for kick, server settings etc.
	 */
	void view_guild_warnings(moderation_command command);

}
//
// Created by arshia on 2/24/23.
//

#pragma once

#include <string_view>

/**
 * @brief namespace mod_action_name - This acts as a string enum.
 */
namespace reactaio::internal::mod_action_name {
	extern inline constexpr std::string_view KICK {"Kick"};
	extern inline constexpr std::string_view BAN{"Ban"};
	extern inline constexpr std::string_view SOFT_BAN{"Soft Ban"};
	extern inline constexpr std::string_view HARD_BAN{"Hard Ban"};
	extern inline constexpr std::string_view WARN{"Warning"};
	extern inline constexpr std::string_view MUTE{"Mute"};
	extern inline constexpr std::string_view UNMUTE{"Unmute"};
	extern inline constexpr std::string_view TIMEOUT{"Timeout"};
	extern inline constexpr std::string_view UNBAN{"Unban"};
	extern inline constexpr std::string_view DELETE_WARN{"Delete warning"};
	extern inline constexpr std::string_view CLEAR_WARNS{"Clear member warning"};
	extern inline constexpr std::string_view CLEAR_GUILD_WARNS{"Clear guild warnings"};
	extern inline constexpr std::string_view LOCK{"Lock channel"};
	extern inline constexpr std::string_view UNLOCK {"Unlock channel"};
	extern inline constexpr std::string_view LOCKDOWN{"Lockdown start"};
	extern inline constexpr std::string_view LOCKDOWN_END{"Lockdown end"};
	extern inline constexpr std::string_view DURATION{"Change command duration"};
	extern inline constexpr std::string_view REASON{"Change action reason"};
	extern inline constexpr std::string_view VIEW_MOD_CASE{"View moderation case"};
	extern inline constexpr std::string_view VIEW_WARNS{"View warnings"};
	extern inline constexpr std::string_view VIEW_AUDIT_LOG{"View audit log entry"};
	extern inline constexpr std::string_view VIEW_BAN_LIST{"View guild ban list"};
} // namespace reactaio::internal::mod_action_name
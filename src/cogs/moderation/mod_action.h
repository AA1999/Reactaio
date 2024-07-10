//
// Created by arshia on 2/24/23.
//

#pragma once

#include <string_view>

namespace reactaio::internal {
	/**
	 * @brief String enum for all moderation actions.
	 */
	struct mod_action_name {

		virtual ~mod_action_name() = 0; // Can't create instances of the class.

		static constexpr std::string_view KICK {"Kick"};
		static constexpr std::string_view BAN{"Ban"};
		static constexpr std::string_view SOFT_BAN{"Soft Ban"};
		static constexpr std::string_view HARD_BAN{"Hard Ban"};
		static constexpr std::string_view WARN{"Warning"};
		static constexpr std::string_view MUTE{"Mute"};
		static constexpr std::string_view UNMUTE{"Unmute"};
		static constexpr std::string_view TIMEOUT{"Timeout"};
		static constexpr std::string_view UNBAN{"Unban"};
		static constexpr std::string_view DELETE_WARN{"Delete warning"};
		static constexpr std::string_view CLEAR_WARNS{"Clear member warning"};
		static constexpr std::string_view CLEAR_GUILD_WARNS{"Clear guild warnings"};
		static constexpr std::string_view LOCK{"Lock channel"};
		static constexpr std::string_view UNLOCK {"Unlock channel"};
		static constexpr std::string_view LOCKDOWN{"Lockdown start"};
		static constexpr std::string_view LOCKDOWN_END{"Lockdown end"};
		static constexpr std::string_view DURATION{"Change command duration"};
		static constexpr std::string_view REASON{"Change action reason"};
		static constexpr std::string_view VIEW_MOD_CASE{"View moderation case"};
		static constexpr std::string_view VIEW_WARNS{"View warnings"};
		static constexpr std::string_view VIEW_AUDIT_LOG{"View audit log entry"};
		static constexpr std::string_view VIEW_BAN_LIST{"View guild ban list"};
	};

} // namespace reactaio::internal::mod_action_name
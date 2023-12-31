//
// Created by arshia on 2/24/23.
//

#pragma once

//#include "../base/aliases.h"

#include "../base/datatypes/fixed_map.h"
#include <initializer_list>

namespace reactaio::internal {
	/**
	 * @brief mod_action_name - A string enum used to insert moderation action names into a SQL query with no typo.
	 * @note This is implemented as a map with fixed size.
	 */
	std::size_t const MOD_ACTION_NAME_SIZE = 20;
	extern inline const reactaio::internal::fixed_map<std::string_view, std::string_view, MOD_ACTION_NAME_SIZE> mod_action_name {
			{"kick", "Kick"},
			{"ban", "Ban"},
			{"softban", "Softban"},
			{"hardban", "hardban"},
			{"warn", "Warning"},
			{"mute", "Mute"},
			{"timeout", "timeout"},
			{"unban", "Unban"},
			{"delwarn", "Delete warn"},
			{"clearwarn", "Clear warnings"},
			{"globalclear", "Guild warnings cleared"},
			{"lock", "Lock channel"},
			{"unlock", "Unlock channel"},
			{"lockdown", "Lockdown start"},
			{"lockdown_end", "Lockdown end"},
			{"duration", "Change action duration"},
			{"reason", "Change action reason"},
			{"view_case", "View moderation case"},
			{"view_warns", "View warnings"},
			{"view_audit", "View audit log entry"},
			{"ban_list", "View guild bans list"}
	};
}
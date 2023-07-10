//
// Created by arshia on 2/24/23.
//

#pragma once

#include "../base/aliases.h"

namespace reactaio::internal {
	struct mod_action_name {
		constexpr static cstring kick = "Kick";
		constexpr static cstring ban = "Ban";
		constexpr static cstring softban = "Soft ban";
		constexpr static cstring hardban = "Hard ban";
		constexpr static cstring warn = "Warning";
		constexpr static cstring mute = "Mute";
		constexpr static cstring timeout = "Timeout";
		constexpr static cstring unban = "Unban";
		constexpr static cstring delwarn = "Delete warn";
		constexpr static cstring clearwarn = "Clear warnings";
		constexpr static cstring globalclear = "Guild warns cleared";
		constexpr static cstring lock = "Lock channel";
		constexpr static cstring unlock = "Unlock channel";
		constexpr static cstring lockdown = "Guild lockdown";
		constexpr static cstring lockdown_end = "Lockdown End";
		constexpr static cstring duration = "Change action duration";
		constexpr static cstring reason = "Change modcase reason";
		constexpr static cstring view_case = "View modcase";
		constexpr static cstring view_warns = "View member warnings";
		constexpr static cstring view_audit = "View audit log entry";
		constexpr static cstring ban_list = "View guild bans list";
	};
}
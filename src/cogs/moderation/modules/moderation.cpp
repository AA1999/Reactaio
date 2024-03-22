//
// Created by arshia on 2/15/23.
//

#include "moderation.h"
#include "../wrappers/kick/kick_wrapper.h"
#include "../wrappers/mute/mute_wrapper.h"
#include "../wrappers/warn/warn_wrapper.h"
#include "../wrappers/warning management/delete_warning.h"
#include "../wrappers/warning management/clear_warnings.h"
#include "../wrappers/warning management/clear_guild_warnings.h"
#include "../wrappers/warning management/view_warnings.h"
#include "../wrappers/warning management/view_guild_warnings.h"
#include "../wrappers/logging/view_modcase.h"
#include "../wrappers/logging/view_muted_members.h"
#include "../wrappers/ban/ban_wrapper.h"
#include "../wrappers/softban/softban_wrapper.h"
#include "../wrappers/hardban/hardban_wrapper.h"
#include "../wrappers/unmute/unmute_wrapper.h"
#include "../wrappers/unban/unban_wrapper.h"
#include "../wrappers/bans/guild_bans_wrapper.h"

#include <string_view>
#include <utility>

namespace reactaio::moderation {

	template <typename T>
	using shared_vector = std::vector<std::shared_ptr<T>>;

	void kick(const std::vector<dpp::guild_member>& members, moderation_command command) {
		kick_wrapper kick_members{members, command};
		kick_members();
	}

	void ban(const std::vector<member_user_variant> &users_or_members, moderation_command command) {
		ban_wrapper ban_users{users_or_members, command};
		ban_users();
	}

	void softban(const std::vector<member_user_variant> &users_or_members, moderation_command command) {
		softban_wrapper softban_users{users_or_members, command};
		softban_users();
	}

	void hardban(const std::vector<member_user_variant> &users_or_members, const moderation_command &command) {
		hardban_wrapper hardban_users(users_or_members, const_cast<moderation_command&>(command));
		hardban_users();
	}

	void mute(const std::vector<dpp::guild_member> &members, moderation_command command) {
		mute_wrapper mute_members{members, command};
		mute_members();
	}

	void warn(const std::vector<dpp::guild_member> &members, moderation_command command) {
		warn_wrapper warn_members{members, command};
		warn_members();
	}

	void delete_warn(warn_t warning_id, moderation_command command) {
		delete_warning delete_warn{std::to_string(warning_id), command};
		delete_warn();
	}

	void clear_member_warnings(dpp::guild_member member, moderation_command command) {
		clear_warnings clear_warns{std::move(member), command};
		clear_warns();
	}

	void delete_all_guild_warns(moderation_command command) {
		clear_guild_warnings clear_guild_warns{command};
		clear_guild_warns();
	}

	void unmute(const std::vector<dpp::guild_member> &members, moderation_command command) {
		unmute_wrapper unmute_members{members, command};
		unmute_members();
	}

	void unban(const shared_vector<dpp::user> &users, moderation_command command) {
		unban_wrapper unban_users{users, command};
		unban_users();
	}

	void view_ban_list(moderation_command command) {
		guild_bans_wrapper view_bans{command};
		view_bans();
	}

	void view_muted_list(moderation_command command) {
		view_muted_members view_muted{command};
		view_muted();
	}

	void view_modase(case_t case_id, moderation_command command) {
		class view_modcase view_moderation_case{std::to_string(case_id), command};
		view_moderation_case();
	}

	void view_member_warnings(dpp::guild_member member, moderation_command command) {
		view_warnings view_warns{std::move(member), command};
		view_warns();
	}

	void view_guild_warnings(moderation_command command) {
		class view_guild_warnings view_guild_warns{command};
		view_guild_warns();
	}
}
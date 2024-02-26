//
// Created by arshia on 2/15/23.
//

#include "moderation.h"
#include "../wrappers/kick/kick_wrapper.h"
#include "../wrappers/mute/mute_wrapper.h"
#include "../wrappers/warn/warn_wrapper.h"
#include "../wrappers/warning management/delete_warning.h"
#include "../wrappers/ban/ban_wrapper.h"
#include "../wrappers/softban/softban_wrapper.h"
#include "../wrappers/hardban/hardban_wrapper.h"


namespace reactaio::moderation {
	void kick(const std::vector<dpp::guild_member>& members, const moderation_command& command) {
		kick_wrapper kick_members{members, const_cast<moderation_command&>(command)};
		kick_members();
	}

	void ban(const std::vector<std::variant<dpp::guild_member, dpp::user *>> &users_or_members, const moderation_command &command) {
		ban_wrapper ban_users{users_or_members, const_cast<moderation_command&>(command)};
		ban_users();
	}
	void softban(const std::vector<std::variant<dpp::guild_member, dpp::user *>> &users_or_members, const moderation_command &command) {
		softban_wrapper softban_users{users_or_members, const_cast<moderation_command&>(command)};
		softban_users();
	}
	void hardban(const std::vector<std::variant<dpp::guild_member, dpp::user *>> &users_or_members, const moderation_command &command) {
		hardban_wrapper hardban_users(users_or_members, const_cast<moderation_command&>(command));
		hardban_users();
	}
	void mute(const std::vector<dpp::guild_member> &members, const moderation_command &command) {
		mute_wrapper mute_members{members, const_cast<moderation_command&>(command)};
		mute_members();
	}
	void warn(const std::vector<dpp::guild_member> &members, const moderation_command &command) {
		warn_wrapper warn_members{members, const_cast<moderation_command&>(command)};
		warn_members();
	}
	void delete_warn(warn_t warning_id, const moderation_command &command) {

	}
	void delete_all_guild_warns(const moderation_command &command) {
	}
	void unmute(const std::vector<dpp::guild_member> &members, const moderation_command &command) {
	}
	void unban(const std::vector<dpp::user *> &users, const moderation_command &command) {
	}
	void view_ban_list(const moderation_command &command) {
	}
	void view_muted_list(const moderation_command &command) {
	}
	void view_warnings(const dpp::guild_member &member, const moderation_command &command) {
	}
	void view_warnings_list(const moderation_command &command) {
	}
}
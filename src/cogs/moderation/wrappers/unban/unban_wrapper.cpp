//
// Created by arshia on 3/12/23.
//

#include "unban_wrapper.h"
#include "../../../base/consts.h"


void unban_wrapper::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;

	process_unbans();
	process_response();
}

void unban_wrapper::check_permissions() {
	auto* bot_user = &command.bot->me;
	auto author_roles = get_member_roles_sorted(command.author);
	auto bot_member = dpp::find_guild_member(command.guild->id, bot_user->id);
	auto* author_top_role = *author_roles.begin();
	auto bot_roles = get_member_roles_sorted(bot_member);
	auto* bot_top_role = *bot_roles.begin();

	if(author_top_role->position < bot_top_role->position) {
		are_errors = true;
		cancel_operation = true;
		errors.emplace_back("❌ Bot top role is below your role. Please move the bot role above the top role");
	}


}

void unban_wrapper::process_unbans() {

}

void unban_wrapper::process_response() {

}
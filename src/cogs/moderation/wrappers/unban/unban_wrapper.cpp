//
// Created by arshia on 3/12/23.
//

#include "unban_wrapper.h"
#include "../../../base/consts.h"
#include "../../../base/helpers.h"
#include "../../mod_action.h"


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

	if(!bot_top_role->has_ban_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ The bot does not have ban_members permission. Please fix this and try again.");
	}

	if(author_top_role->position < bot_top_role->position) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot top role is below your role. Please move the bot role above the top role");
	}

	auto transaction = pqxx::transaction{*command.connection};
	auto hard_bans_query = transaction.exec_prepared("hardban_get", std::to_string(command.guild->id));
	transaction.abort();

	std::vector<std::string> hard_bans;
	for(auto const& row: hard_bans_query)
		hard_bans.push_back(row["user_id"].as<std::string>());
	std::vector<std::string> user_ids;
	std::ranges::transform(users.begin(), users.end(), std::back_inserter(user_ids), [](dpp::user const* user){
		return std::to_string(user->id);
	});



}

void unban_wrapper::process_unbans() {

	for(auto* user: users) {
		command.bot->set_audit_reason(std::string(command.reason)).guild_ban_delete(command.guild->id, user->id, [this, user](auto const& completion){
			if(completion.is_error()) {
				auto error = completion.get_error();
				errors.emplace_back(std::format("❌ Error {}: {}", error.code, error.message));
				users_with_errors.push_back(user);
				return;
			}
			auto transaction = pqxx::transaction{*command.connection};
			auto max_query	 = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
			auto max_id = std::get<0>(max_query.as<case_t>()) + 1;
			transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id,
									  reactaio::internal::mod_action_name["unban"], std::to_string(command.author.user_id), std::to_string(user->id),
									  command.reason);
			transaction.commit();
			transaction.abort();
		});
	}
}

void unban_wrapper::process_response() {

}
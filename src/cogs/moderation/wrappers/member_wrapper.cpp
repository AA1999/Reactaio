//
// Created by arshia on 12/20/24.
//

#include "member_wrapper.h"

#include "../../core/algorithm.h"


void member_wrapper::log_modcase(std::string_view const& command_name) const {
	auto transaction = pqxx::work{*command.connection};
	auto const max_query = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));

	auto const max_id = max_query["case_id"].is_null() ? 1 : max_query["case_id"].as<std::uint64_t>() + 1;
	shared_vector<dpp::guild_member> members_with_no_errors;
	reactaio::set_difference(members, members_with_errors, members_with_no_errors);
	if(duration) {
		for(auto const& member: members_with_no_errors)
			transaction.exec_prepared("modcase_insert_duration", std::to_string(command.guild->id), max_id, command_name, duration->to_string(),
								  std::to_string(command.author->user_id), std::to_string(member->user_id), command.reason);
		transaction.commit();
		return;
	}
	for(auto const& member: members_with_no_errors)
		transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id, command_name,
							  std::to_string(command.author->user_id), std::to_string(member->user_id), command.reason);
	transaction.commit();
}

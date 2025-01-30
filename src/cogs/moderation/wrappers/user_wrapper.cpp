//
// Created by arshia on 1/29/25.
//

#include "user_wrapper.h"
#include "algorithm.h"


void user_wrapper::log_modcase(std::string_view const& command_name) const {
	auto transaction = pqxx::work{*command.connection};
	auto const max_query = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
	auto const max_id = max_query["case_id"].is_null() ? 1 : max_query["case_id"].as<std::uint64_t>() + 1;
	shared_vector<dpp::user> users_with_no_errors;

	reactaio::set_difference(users, users_with_errors, users_with_no_errors);
	for(auto const& user: users_with_no_errors)
		transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id, command_name,
							  std::to_string(command.author->user_id), std::to_string(user->id), command.reason);
	transaction.commit();
}

//
// Created by arshia on 1/3/25.
//

#include "hybrid_wrapper.h"
#include "../../core/algorithm.h"


void hybrid_wrapper::log_modcase(std::string_view const& command_name) const {
	auto transaction = pqxx::work{*command.connection};
	auto const max_query = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
	auto const max_id = max_query["case_id"].is_null() ? 1 : max_query["case_id"].as<std::uint64_t>() + 1;

	shared_vector<dpp::user> all_users;
	shared_vector<dpp::user> users_with_no_errors;

	reactaio::transform(snowflakes, all_users, [&](std::variant<member_ptr, user_ptr> const& user) {
		if(auto const member = std::get_if<member_ptr>(&user))
			return std::make_shared<dpp::user>(*(*member)->get_user());
		return std::get<user_ptr>(user);
	});

	reactaio::set_difference(all_users, users_with_errors, users_with_no_errors);

	if(duration) {
		for (auto const& user: users_with_no_errors)
			transaction.exec_prepared("modcase_insert_duration", std::to_string(command.guild->id), max_id, command_name, duration->to_string(),
							  std::to_string(command.author->user_id), std::to_string(user->id), command.reason);
		transaction.commit();
		return;
	}

	for(auto const& user: users_with_no_errors)
		transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id, command_name,
							  std::to_string(command.author->user_id), std::to_string(user->id), command.reason);
	transaction.commit();
}

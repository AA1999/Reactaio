//
// Created by arshia on 2/16/23.
//

#include "command_wrapper.h"

#include "../../core/algorithm.h"
#include "../../core/consts.h"
#include "../../core/discord/message_paginator.h"

[[nodiscard]] std::optional<dpp::message> command_wrapper::error() const {
	if (!has_error())
		return std::nullopt;
	return error_message;
}

void command_wrapper::operator()() {
	wrapper_function();
}


void command_wrapper::log_command_invoke(std::string_view const& name) const {
	pqxx::work transaction{*command.connection};
	transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author->user_id), name,
							dpp::utility::current_date_time());
	transaction.commit();
}


void command_wrapper::invoke_error_webhook() {
	pqxx::work transaction{*command.connection};
	auto const webhook_url_query = transaction.exec_prepared1("get_error_webhook", std::to_string(command.guild->id));
	transaction.commit();

	error_message.content.clear();

	auto const format_split = join_with_limit(errors, bot_max_embed_chars);

	if (format_split.size() == 1)
		error_message.set_content(format_split.front());
	else {
		message_paginator paginator{error_message, format_split, command};
	}

	if (!webhook_url_query["bot_error_logs"].is_null()) {
		auto const webhook_url = webhook_url_query["bot_error_logs"].as<std::string>();
		dpp::webhook const error_webhook{webhook_url};
		command.bot->execute_webhook(error_webhook, error_message);
		return;
	}
	auto const full_error_message = std::format("The server hasn't set a channel for bot errors, so the errors are being sent to your DMs: \n\n\n {}",
												error_message);
	error_message.set_content(full_error_message);
	command.bot->direct_message_create(command.author->user_id, error_message);
}

shared_vector<dpp::role> command_wrapper::get_protected_roles() const {
	pqxx::work transaction{*command.connection};
	auto const protected_roles_row = transaction.exec_prepared1("protected_roles", std::to_string(command.guild->id));
	if (protected_roles_row["protected_roles"].is_null())
		return {};
	auto const protected_roles_array = protected_roles_row["protected_roles"].as_sql_array<dpp::snowflake>();
	shared_vector<dpp::role> roles{protected_roles_array.size()};
	reactaio::transform(protected_roles_array, roles, [](dpp::snowflake const &role_id) {
		return std::make_shared<dpp::role>(*find_role(role_id));
	});

	return roles;
}

shared_vector<dpp::role> command_wrapper::get_permitted_roles(std::string_view const& command_name) const {
	pqxx::work transaction{*command.connection};
	auto const mod_role_query = transaction.exec_prepared("get_mod_perm_roles", std::to_string(command.guild->id), command_name);
	transaction.commit();
	if(mod_role_query.empty())
		return {};
	shared_vector<dpp::role> roles(mod_role_query.size());
	reactaio::transform(mod_role_query, roles, [](pqxx::row const& row) {
		return std::make_shared<dpp::role>(*find_role(row["role_id"].as<dpp::snowflake>()));
	});

	return roles;
}

//
// Created by arshia on 2/16/23.
//

#include "helpers.h"

#include "algorithm.h"
#include "strings.h"

#include <string>
#include <sstream>
#include <chrono>

std::vector<std::string> join_with_limit(const std::vector<std::string> &vector, std::size_t length) {
	std::vector<std::string> result;
	std::size_t current_size{0};
	std::string to_send;
	to_send.reserve(length);
	for(auto const& string: vector) {
		if(string.length() + current_size + sizeof('\n') <= length) {
			to_send.append(string).append("\n");
			current_size += string.length();
		}
		else if(string.length() <= length) {
			result.push_back(to_send);
			to_send.clear();
			to_send.append(string).append("\n");
			current_size = string.length();
		}
		if(&string == &vector.back())
			result.push_back(to_send);
	}
	return result;
}


std::vector<std::string_view> find_all_of(std::string_view const string, std::string_view const find) {
	std::vector<std::string_view> results;
	std::size_t position{0};
	while ((position = string.find(find, position)) != std::string_view::npos) {
		results.push_back(string.substr(position, find.length()));
		position += find.length();
	}
	return results;
}

std::set<std::size_t> find_index_all(std::string_view string, std::string_view find) {
	std::set<std::size_t> positions;
	std::size_t position{0};
	while((position = string.find(find, position)) != std::string_view::npos) {
		positions.insert(position);
		position += find.length();
	}
	return positions;
}

std::vector<std::string> get_tokens(std::string_view const string) {
	std::string str{string};
	str.append(" ");
	std::vector<std::string> result{};
	uint start{};
	for (uint i{0}; i < str.length(); ++i) {
		if (((std::isalnum(str[i]) != 0) && (std::isspace(str[i + 1]) != 0))	   // `a ` or `1 `
			|| ((std::isdigit(str[i]) != 0) && (std::isalpha(str[i + 1]) != 0))	   // `a1`
			|| ((std::isalpha(str[i]) != 0) && (std::isdigit(str[i + 1]) != 0))) { // `1a`
			result.push_back(str.substr(start, i - start + 1));
			start = i + 1;
		}
		if (((std::isspace(str[i]) != 0) && (std::isalnum(str[i + 1]) != 0))	   // ` 1` or ` a`
			|| ((std::isalpha(str[i]) != 0) && (std::isdigit(str[i + 1]) != 0))	   // `a1`
			|| ((std::isdigit(str[i]) != 0) && (std::isalpha(str[i + 1]) != 0))) { // `1a`
			start = i + 1;
		}
	}
	return result;
}

std::optional<reactaio::internal::duration> parse_human_time(std::string_view const string) {
	reactaio::internal::duration res{};
	auto const tokens = get_tokens(remove_non_alphanumeric(string));

	if (tokens.size() % 2 == 0) // size must be even
		return std::nullopt;

	for (uint i{0}; i < tokens.size(); ++i) {
		if (is_all_digit(tokens.at(2UL * i)) && !is_all_digit(tokens.at(2UL * i + 1))) {
			auto const number = std::stoull(tokens.at(2UL * i));
			auto const& unit = tokens.at(2 * i + 1);
			for (uint j{0}; j < reactaio::internal::units.size(); ++j) {
				auto [u, names] = reactaio::internal::units.at(j);
				if (contains(names, unit))
					res.values.at(j) += static_cast<uint64_t>(number);
			}
		}
		else
			// one of the values isn't of the correct format
			// eg: 2 numbers, or 2 words in a row
			return std::nullopt;
	}
	return res;
}

shared_vector<dpp::role> get_roles_sorted(const guild_ptr &guild, bool descending) {
	shared_vector<dpp::role> roles;
	auto guild_roles = guild->roles;
	roles.reserve(guild_roles.size());
	for (auto const &role_id: guild_roles)
		roles.insert(dpp::find_role(role_id));
	if(descending)
		roles.reverse();
	return roles;
}

shared_vector<dpp::role> get_roles_sorted(const dpp::guild_member &member, bool descending) {
	shared_vector<dpp::role> roles;
	auto member_roles = member.get_roles();
	roles.reserve(member_roles.size());
	for (auto const &role_id: member_roles)
		roles.insert(dpp::find_role(role_id));
	if (descending)
		roles.reverse();
	return roles;
}

shared_vector<dpp::role> get_guild_protected_roles(discord_command const& command) {
	pqxx::work transaction{*command.connection};
	auto const protected_roles_row = transaction.exec_prepared1("protected_roles", std::to_string(command.guild->id));
	if (protected_roles_row["protected_roles"].is_null())
		return {};
	shared_vector<dpp::role> roles;
	auto const protected_roles_array = protected_roles_row["protected_roles"].as_sql_array<dpp::snowflake>();
	reactaio::transform(protected_roles_array, roles, [](dpp::snowflake const &role_id) {
		return std::make_shared<dpp::role>(*find_role(role_id));
	});

	return roles;
}

shared_vector<dpp::role> get_mod_perm_roles(discord_command const& command, const std::string_view& command_name) {
	pqxx::work transaction{*command.connection};
	auto const mod_role_query = transaction.exec_prepared("get_mod_perm_roles", std::to_string(command.guild->id), command_name);
	if(mod_role_query.empty())
		return {};
	shared_vector<dpp::role> roles;
	reactaio::transform(mod_role_query, roles, [](pqxx::row const& row) {
		return std::make_shared<dpp::role>(*find_role(row["role_id"].as<dpp::snowflake>()));
	});

	return roles;
}

std::chrono::time_point<std::chrono::system_clock> parse_psql_timestamp(std::string_view const timestamp, std::string_view const format) {
	std::chrono::sys_seconds time_point;
	std::istringstream parser{std::string{timestamp}};
	parser >> parse(std::string{format}, time_point);
	return std::chrono::system_clock::from_time_t(time_point.time_since_epoch().count());
}
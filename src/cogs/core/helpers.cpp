//
// Created by arshia on 2/16/23.
//

#include "helpers.h"
#include "strings.h"

#include <algorithm>
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
	while((position = string.find(find, position)) != std::string_view::npos) {
		results.push_back(string.substr(position, find.length()));
		position += find.length();
	}
	return results;
}

std::vector<std::size_t> find_index_all(std::string_view string, std::string_view find) {
	std::vector<std::size_t> positions;
	std::size_t position{0};
	while((position = string.find(find, position)) != std::string_view::npos) {
		positions.push_back(position);
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
	auto tokens = get_tokens(remove_non_alphanumeric(string));

	if (tokens.size() % 2 == 0) // size must be even
		return std::nullopt;

	for (uint i{0}; i < tokens.size(); ++i) {
		if (is_all_digit(tokens.at(2UL * i)) && !is_all_digit(tokens.at(2UL * i + 1))) {
			auto const number = std::stoull(tokens.at(2UL * i));
			auto const unit	  = tokens.at(2 * i + 1);
			for (uint j{0}; j < reactaio::internal::units.size(); ++j) {
				auto [u, names] = reactaio::internal::units.at(j);
				if (contains(names, unit))
					res.values.at(j) += (int)number;
			}
		}
		else
			// one of the values isn't of the correct format
			// eg: 2 numbers, or 2 words in a row
			return std::nullopt;
	}
	return res;
}

std::vector<dpp::role*> get_roles_sorted(dpp::guild* guild, bool descending) {
	std::vector<dpp::role*> roles;
	auto guild_roles = guild->roles;
	roles.reserve(guild_roles.size());
	for(auto const& role_id: guild_roles)
		roles.emplace_back(dpp::find_role(role_id));
	if(descending) {
		std::ranges::sort(roles, std::ranges::greater{}, [](dpp::role* role){
			return role->position;
		});
	}
	else {
		std::ranges::sort(roles, std::ranges::less{}, [](dpp::role* role){
			return role->position;
		});
	}
	return roles;
}

std::vector<dpp::role*> get_roles_sorted(const dpp::guild_member& member, bool descending) {
	std::vector<dpp::role*> roles;
	auto member_roles = member.get_roles();
	roles.reserve(member_roles.size());
for(auto const& role_id: member_roles)
		roles.emplace_back(dpp::find_role(role_id));
	if(descending) {
		std::ranges::sort(roles, std::ranges::greater{}, [](dpp::role* role){
			return role->position;
		});
	}
	else {
		std::ranges::sort(roles, std::ranges::less{}, [](dpp::role* role){
			return role->position;
		});
	}
	return roles;
}

std::chrono::time_point<std::chrono::system_clock> parse_psql_timestamp(std::string_view const timestamp, std::string_view const format) {
	std::istringstream parser{std::string{timestamp}};
	std::tm time{};
	parser >> std::get_time(&time, std::string{format}.c_str());
	auto const as_time_t{std::mktime(&time)};
	return std::chrono::system_clock::from_time_t(as_time_t);
}
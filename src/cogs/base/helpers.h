//
// Created by arshia on 2/16/23.
//

#pragma once

#include "aliases.h"
#include "datatypes/duration.h"
#include "datatypes/parser_item.h"

#include <dpp/dpp.h>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <pqxx/pqxx>
#include <sstream>
#include <format>

template <typename T>
requires std::ranges::range<T>
bool includes(const T& vector, const typename T::value_type& key) {
	return std::find(std::ranges::begin(vector), std::ranges::end(vector), key) != std::ranges::end(vector);
}

bool includes(std::string_view string, std::string_view find);
std::vector<std::string_view> find_all_of(std::string_view string, std::string_view find);
std::vector<std::size_t> find_index_all(std::string_view string, std::string_view find);

bool is_all_digit(std::string_view string);
std::string remove_non_alnum(std::string_view string);
member_t non_bot_members(dpp::guild* guild);
member_t bot_members(dpp::guild* guild);
constexpr std::string ordinal(ullong number) {
	ushort const last_digit = number % 10;
	if (last_digit == 1 && number != 11)
		return std::format("{}st", number);
	if (last_digit == 2 && number != 12)
		return std::format("{}nd", number);
	if (last_digit == 3 && number != 13)
		return std::format("{}rd", number);
	return std::format("{}th", number);
}

constexpr std::string join(const std::vector<std::string>& vector, std::string_view separator) {
	std::string result;
	if (vector.empty())
		return result;

	for (auto const& element : vector)
		if(result.empty())
			result.append(element);
		else
			result.append(separator).append(element);
	return result;

}
std::vector<std::string> join_with_limit(const std::vector<std::string>& vector, std::size_t length);

std::vector<std::string> get_tokens(std::string_view string);
std::optional<duration> parse_human_time(std::string_view string);

template <typename T>
std::vector<T> parse_psql_array(const pqxx::field& field) {
	std::vector<T> result;
	auto array_parser = field.as_array();
	parser_item item = parser_item{array_parser.get_next()};
	while(item.juncture != pqxx::array_parser::juncture::done) {
		result.push_back(static_cast<T>(item.value));
		item = parser_item{array_parser.get_next()};
	}
	return result;
}

std::vector<dpp::role*> get_guild_sorted_roles(dpp::guild* guild, dpp::guild_member member, bool descending = true);
std::vector<dpp::role*> get_member_roles_sorted(const dpp::guild_member& member, bool descending = true);
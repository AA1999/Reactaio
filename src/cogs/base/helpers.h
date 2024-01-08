//
// Created by arshia on 2/16/23.
//

#pragma once

#include "aliases.h"
#include "datatypes/duration.h"
#include "datatypes/parser_item.h"

#include <dpp/dpp.h>
#include <optional>
#include <string>
#include <string_view>
#include <pqxx/pqxx>
#include <format>
#include <ranges>
#include <vector>

/**
 * @brief includes - Whether if the specific element exists in the map.
 * @tparam T Type of the vector.
 * @param vector The vector to look the element up.
 * @param key The element to look up.
 * @return true if the element exists
 * @return false otherwise
 */
template <typename T>
requires std::ranges::range<T>
bool includes(const T& vector, const typename T::value_type& key) {
	return std::find(std::ranges::begin(vector), std::ranges::end(vector), key) != std::ranges::end(vector);
}

/**
 * @brief includes - Whether if a string can be found inside another string
 * @param string String to look up the substring inside.
 * @param find The substring to find.
 * @return true if the string is found
 * @return false otherwise
 */
bool includes(std::string_view string, std::string_view find);


/**
 * @brief common_element - Finds the common element between two ranges.
 * @tparam A Type of the first range.
 * @tparam B Type of the second range.
 * @param range_a First range
 * @param range_b Second range
 * @return The common element if found. An empty std::optional object if not
 */
template <std::ranges::input_range A, std::ranges::input_range B>
std::optional<std::ranges::range_value_t<A>> common_element(A const&& range_a, B const&& range_b) {
	auto it = std::ranges::find_first_of(std::forward<A>(range_a), std::forward<B>(range_b));
	return it != it.end() ? *it : std::optional<std::ranges::range_value_t<A>>{};
}

/**
 * @brief find_all_off - Find all instances of a string inside another string.
 * @param string The string to find the substring inside.
 * @param find The substring to find.
 * @return a std::vector finding all the substrings that occur
 */
std::vector<std::string_view> find_all_of(std::string_view string, std::string_view find);

/**
 * @brief find_index_all - Finds all indexes of a substring occurrence in a string.
 * @param string The string to find the occurrences inside
 * @param find The substring to find.
 * @return A std::vector of all the indexes with the substring match.
 */
std::vector<std::size_t> find_index_all(std::string_view string, std::string_view find);

/**
 * @brief is_all_digit - Checks if a string is all numeric.
 * @param string The string to check.
 * @return true if the string is all numeric (e.g "1257")
 * @return false otherwise
 */
bool is_all_digit(std::string_view string);

/**
 * @brief remove_non_alphanumeric - Removes all non-numeric and alphabetic characters (0-9, Aa-Zz) from a string.
 * @param string The string to have the characters removed from.
 * @return The resulting string with only 0-9 and Aa-zZ characters.
 */
std::string remove_non_alphanumeric(std::string_view string);

/**
 * @brief non_bot_members - Counts all the dpp::guild_members from a guild if they're not a bot account.
 * @note This function won't and cannot count the self-bot accounts as a bot.
 * @param guild The guild to count the non-bot members in.
 * @return The count of the members that aren't bots.
 */
member_t non_bot_members(dpp::guild* guild);

/**
 * @brief bot_members - Counts all the dpp::guild_members from a guild if they're a bot account
 * @param guild The guild to count the bot members in.
 * @return The count of the members that are bots.
 */
member_t bot_members(dpp::guild* guild);

/**
 * @brief ordinal - Returns the ordinal format of a number
 * @param number The number given.
 * @return The ordinal format of the number.
 */
constexpr std::string ordinal(ullong number) {
	ushort const last_digit = number % 10;
	ushort const last_two_digits = number % 100;
	if (last_digit == 1 && last_two_digits != 11)
		return std::format("{}st", number);
	if (last_digit == 2 && last_two_digits != 12)
		return std::format("{}nd", number);
	if (last_digit == 3 && last_two_digits != 13)
		return std::format("{}rd", number);
	return std::format("{}th", number);
}


/**
 * @brief join -
 * @param vector The vector to join.
 * @param separator The separator to add to the end of each element.
 * @return A string created from concatenating all the vector elements.
 */
constexpr std::string join(const std::vector<std::string>& vector, std::string_view separator) {
	if(vector.empty())
		return "";
	std::string result;
	for(auto const& element : vector)
		if(result.empty())
			result.append(element);
		else
			result.append(separator).append(element);
	return result;

}

/**
 * @brief join_with_limit - Makes sized slices of a vector.
 * @param vector The vector to be sliced.
 * @param length The length limit.
 * @return
 */
std::vector<std::string> join_with_limit(const std::vector<std::string>& vector, std::size_t length);


/**
 * @brief get_tokens - Tokenizer for converting a string into a time format.
 * @param string The string to tokenize.
 * @return A vector of tokenized day/week etc. formats
 */
std::vector<std::string> get_tokens(std::string_view string);

/**
 * @brief parse_human_time Converts the string format into a time format.
 * @param string The string to be parsed.
 * @return Equivalent time format. For example 2d -> 2 days.
 */
std::optional<duration> parse_human_time(std::string_view string);


/**
 * @brief parse_sql_array - Parses a SQL query result into a proper array.
 * @tparam T Type of the resulting array.
 * @param field
 * @return
 */
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

/**
 * @brief get_guild_sorted_roles - Returns a sorted vector of a guild's roles.
 * @param guild The guild to fetch the roles from.
 * @param descending Whether to sort them by descending or ascending.
 * @return A sorted vector of guild roles.
 */
std::vector<dpp::role*> get_guild_sorted_roles(dpp::guild* guild, bool descending = true);

/**
 * @brief get_member_roles_sorted - Returns a sorted vector of a dpp::guild_member's roles.
 * @param member The member to fetch the roles from.
 * @param descending Whether to sort them by descending or ascending.
 * @return A sorted vector of member roles.
 */
std::vector<dpp::role*> get_member_roles_sorted(const dpp::guild_member& member, bool descending = true);
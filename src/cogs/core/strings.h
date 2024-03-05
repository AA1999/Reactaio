//
// Created by arshia on 1/23/24.
//

#pragma once

#include <string>
#include <algorithm>
#include <ranges>

/**
 * @brief is_all_digit - Checks if a string is all numeric.
 * @param string The string to check.
 * @return true if the string is all numeric (e.g "1257")
 * @return false otherwise
 */
constexpr bool is_all_digit(std::string_view string) {
	return std::ranges::all_of(string, ::isdigit);
}

/**
 * @brief remove_non_alphanumeric - Removes all non-numeric and alphabetic characters (0-9, Aa-Zz) from a string.
 * @param string The string to have the characters removed from.
 * @return The resulting string with only 0-9 and Aa-zZ characters.
 */
constexpr std::string remove_non_alphanumeric(std::string_view string) {
	std::string result;
	std::ranges::copy_if(string, std::back_inserter(result), ::isalnum);
	return result;
}

/**
 * @brief remove_alpha - Removes all alphabetic (Aa-Zz) characters from a string.
 * @param string The string to remove the characters from.
 * @return A copy that doesn't have the alphabetic characters.
 */
constexpr std::string remove_alpha(std::string_view string) {
	std::string result;
	std::ranges::remove_copy_if(string, std::back_inserter(result), ::isalpha);
	return result;
}

/**
 * @brief remove_numeric - Removes all numeric (0-9) characters from a string.
 * @param string The string to remove the characters from.
 * @return A copy that doesn't have the numeric characters.
 */
constexpr std::string remove_numeric(std::string_view string) {
	std::string result;
	std::ranges::remove_copy_if(string, std::back_inserter(result), ::isdigit);
	return result;
}


/**
 * @brief Splits a string into a range of string views.
 * @tparam CharT Encoding
 * @param string The string to split.
 * @param delimiter The delimiter to have the string split upon reaching it.
 * @return A range of string_view each split.
 */
template <typename CharT>
constexpr auto split(std::basic_string_view<CharT> string, std::basic_string_view<CharT> delimiter) {
	return std::views::split(string, delimiter) | std::views::transform([](auto word) { return std::string_view(std::ranges::begin(word), std::ranges::end(word)); });
}

/**
 * @brief  to_lowercase - Converts a string to lowercase. Example: "AdFs" -> "adfs"
 * @param string The string to convert to lowercase.
 * @return
 */
constexpr std::string to_lowercase(std::string const& string) {
	std::string result;
	std::ranges::transform(string, std::back_inserter(result), ::tolower);
	return result;
}

/**
 * @brief Converts a string into uppercase
 * @param string The string to convert to upper case.
 * @return An uppercase copy of the string.
 */
constexpr std::string to_uppercase(std::string_view const string) {
	std::string result;
	std::ranges::transform(string, std::back_inserter(result), ::toupper);
	return result;
}

/**
 * @brief Converts a string into uppercase ("hello world" -> "Hello World")
 * @param string The string to convert to title case.
 * @return A title case copy of the string.
 */
constexpr std::string to_titlecase(std::string string) {
	for(bool word_start{true}; auto& character: string) {
		character = word_start ? static_cast<char>(std::toupper(static_cast<unsigned char>(character))) : static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
		word_start = !std::isspace(character);
	}
	return string;
}
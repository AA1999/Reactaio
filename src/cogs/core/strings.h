//
// Created by arshia on 1/23/24.
//

#pragma once

#include "datatypes/split_proxy.h"

#include <string>
#include <algorithm>

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
 *
 * @tparam CharT Encoding.
 * @param string The string to split.
 * @param delimiter The delimiter to have the string split when it sees that.
 * @return A container that contains the split string.
 */
template <typename CharT>
constexpr reactaio::internal::split_proxy<CharT> split(std::basic_string_view<CharT> string, std::basic_string_view<CharT> delimiter) {
	return reactaio::internal::split_proxy{string, delimiter};
}

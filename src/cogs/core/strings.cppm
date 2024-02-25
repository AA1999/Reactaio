//
// Created by arshia on 2/25/24.
//

#include <algorithm>
#include <string>
#include <string_view>

export module strings;

import split_proxy;

export {
	/**
	 * @brief is_all_digit - Checks if a string is all numeric.
	 * @param string The string to check.
	 * @return true if the string is all numeric (e.g "1257")
	 * @return false otherwise
	 */
	bool is_all_digit(std::string_view string) {
		return std::ranges::all_of(string, ::isdigit);
	}

	/**
	 * @brief remove_non_alphanumeric - Removes all non-numeric and alphabetic characters (0-9, Aa-Zz) from a string.
	 * @param string The string to have the characters removed from.
	 * @return The resulting string with only 0-9 and Aa-zZ characters.
	 */
	std::string remove_non_alphanumeric(std::string_view string);

	/**
	 * @brief remove_alpha - Removes all alphabetic (Aa-Zz) characters from a string.
	 * @param string The string to remove the characters from.
	 * @return A copy that doesn't have the alphabetic characters.
	 */
	std::string remove_alpha(std::string_view string);

	/**
	 * @brief remove_numeric - Removes all numeric (0-9) characters from a string.
	 * @param string The string to remove the characters from.
	 * @return A copy that doesn't have the numeric characters.
	 */
	std::string remove_numeric(std::string_view string);

	/**
	 * @brief split - Splits a string.
	 * @tparam CharT Encoding of the strings
	 * @param string The string to be split.
	 * @param delimiter The delimiter to split when found in the string.
	 * @return A container for the split strings.
	 */
	template<typename CharT>
	reactaio::internal::split_proxy<CharT> split(std::basic_string_view<CharT> string, std::basic_string_view<CharT> delimiter) {
		return reactaio::internal::split_proxy{string, delimiter};
	}
}
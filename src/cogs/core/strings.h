//
// Created by arshia on 1/23/24.
//

#pragma once

#include <string_view>
#include <vector>

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
 * @brief split - Splits a string into a vector by the specified delimiter.
 * @param string The string to split.
 * @param delimiter
 * @return
 */
std::vector<std::string> split(std::string string, std::string_view delimiter = " ");
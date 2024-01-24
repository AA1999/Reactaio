//
// Created by arshia on 1/23/24.
//

#include "strings.h"

#include <algorithm>
#include <string>
#include <cctype>
#include <ranges>

bool is_all_digit(std::string_view string) {
	return std::ranges::all_of(string, ::isdigit);
}

std::string remove_non_alphanumeric(std::string_view string) {
	std::string result;
	std::ranges::copy_if(string, std::back_inserter(result), ::isalnum);
	return result;
}

std::string remove_alpha(std::string_view string) {
	std::string result;
	std::ranges::copy_if(string, std::back_inserter(result), [](auto const& character){
		return !std::isalpha(character);
	});
	return result;
}

std::string remove_numeric(std::string_view string) {
	std::string result;
	std::ranges::copy_if(string, std::back_inserter(result), [](auto const& character) {
		return !std::isdigit(character);
	});
	return result;
}

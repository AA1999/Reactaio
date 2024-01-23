//
// Created by arshia on 1/23/24.
//

#include "strings.h"

#include <algorithm>
#include <string>
#include <cctype>

bool is_all_digit(std::string_view string) {
	return std::ranges::all_of(string, ::isdigit);
}

std::string remove_non_alphanumeric(std::string_view string) {
	std::string result;
	std::ranges::copy_if(string, std::back_inserter(result), ::isalnum);
	return result;
}

std::vector<std::string> split(std::string string, std::string_view delimiter) {
	std::vector<std::string> result;
	std::size_t pos{0};
	while((pos = string.find(delimiter))) {
		result.push_back(string.substr(0, pos));
		string.erase(0, pos + delimiter.length());
	}
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

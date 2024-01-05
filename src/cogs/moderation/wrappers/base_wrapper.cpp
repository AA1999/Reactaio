//
// Created by arshia on 2/16/23.
//

#include "base_wrapper.h"

bool base_wrapper::has_error() const {
	return !errors.empty();
}


std::optional<dpp::message> base_wrapper::error() const {
	if (!has_error())
		return std::nullopt;
	return error_message;
}

void base_wrapper::operator()() {
	wrapper_function();
}



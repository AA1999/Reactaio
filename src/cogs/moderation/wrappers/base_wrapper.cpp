//
// Created by arshia on 2/16/23.
//

#include "base_wrapper.h"

bool base_wrapper::is_error() const {
	return are_errors;
}


std::optional<dpp::message> base_wrapper::error() const {
	if (!is_error())
		return std::nullopt;
	return error_message;
}

void base_wrapper::operator()() {
	wrapper_function();
}



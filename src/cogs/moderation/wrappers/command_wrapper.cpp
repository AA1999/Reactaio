//
// Created by arshia on 2/16/23.
//

#include "command_wrapper.h"


bool command_wrapper::has_error() const {
	return !errors.empty();
}

std::optional<dpp::message> command_wrapper::error() const {
	if (!has_error())
		return std::nullopt;
	return error_message;
}

void command_wrapper::operator()() {
	wrapper_function();
}



//
// Created by arshia on 2/16/23.
//

#include "base_wrapper.h"
#include "../../../exceptions/not_implemented.h"

void base_wrapper::wrapper_function() {
	throw not_implemented{};
}

bool base_wrapper::is_error() const {
	return are_errors;
}

bool base_wrapper::are_all_errors() const {
	throw not_implemented{};
}

std::optional<dpp::message> base_wrapper::error() const {
	if (!is_error())
		return std::nullopt;
	return error_message;
}

void base_wrapper::operator()() {
	wrapper_function();
}

void base_wrapper::check_permissions() {
	throw not_implemented();
}


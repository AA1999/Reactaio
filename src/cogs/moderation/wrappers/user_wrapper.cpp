//
// Created by arshia on 3/12/23.
//

#include "user_wrapper.h"
#include "../../../exceptions/not_implemented.h"

void user_wrapper::check_permissions() {
	throw not_implemented{};
}

void user_wrapper::wrapper_function() {
	throw not_implemented{};
}

bool user_wrapper::are_all_errors() const {
	return users_with_errors.size() == users.size();
}


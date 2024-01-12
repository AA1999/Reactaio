//
// Created by arshia on 3/12/23.
//

#include "member_wrapper.h"
#include "../../../exceptions/not_implemented.h"

void member_wrapper::check_permissions() {
	throw not_implemented{};
}

void member_wrapper::wrapper_function() {
	throw not_implemented{};
}

bool member_wrapper::are_all_errors() const {
	return members_with_errors.size() == members.size();
}
//
// Created by arshia on 3/12/23.
//

#include "hybrid_wrapper.h"
#include "../../../exceptions/not_implemented.h"

void hybrid_wrapper::check_permissions() {
	throw not_implemented{};
}

void hybrid_wrapper::wrapper_function() {
	throw not_implemented{};
}

bool hybrid_wrapper::are_all_errors() const {
	return users_with_error.size() == snowflakes.size();
}


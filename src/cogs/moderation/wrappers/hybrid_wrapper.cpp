//
// Created by arshia on 3/12/23.
//

#include "hybrid_wrapper.h"

bool hybrid_wrapper::are_all_errors() const {
	return users_with_errors.size() == snowflakes.size();
}


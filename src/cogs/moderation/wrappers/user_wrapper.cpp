//
// Created by arshia on 3/12/23.
//

#include "user_wrapper.h"

bool user_wrapper::are_all_errors() const {
	return users_with_errors.size() == users.size();
}


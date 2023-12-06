//
// Created by arshia on 3/12/23.
//

#include "member_wrapper.h"

bool member_wrapper::are_all_errors() const {
	return members_with_errors.size() == members.size();
}
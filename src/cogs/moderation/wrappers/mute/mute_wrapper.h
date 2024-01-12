//
// Created by arshia on 3/4/23.
//

#pragma once

#include "../member_wrapper.h"

#include <dpp/dpp.h>
#include <vector>

class mute_wrapper: public member_wrapper {
	void wrapper_function() override;
	void check_permissions() override;

	void process_mutes();
	void process_response();

	bool use_timeout{true};

public:
	using member_wrapper::member_wrapper;
};

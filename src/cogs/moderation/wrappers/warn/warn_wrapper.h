//
// Created by arshia on 2/25/23.
//

#pragma once

#include "../member_wrapper.h"

class warn_wrapper: public member_wrapper {
	void wrapper_function() override;
	void check_permissions() override;

	void process_warnings();
	void process_response();
public:
	using member_wrapper::member_wrapper;
};

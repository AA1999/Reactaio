//
// Created by arshia on 2/15/23.
//

#pragma once
#include "../member_wrapper.h"

#include <dpp/dpp.h>
#include <utility>
#include <vector>

class kick_wrapper: public member_wrapper {
	void wrapper_function() override;
	void check_permissions() override;

	void process_kicks();
	void process_response();

  public:
	using member_wrapper::member_wrapper;
};

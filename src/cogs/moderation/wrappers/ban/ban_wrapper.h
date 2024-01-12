//
// Created by arshia on 2/24/23.
//

#pragma once

#include "../hybrid_wrapper.h"


class ban_wrapper: public hybrid_wrapper {
	void wrapper_function() override;
	void check_permissions() override;

	void process_bans();
	void process_response();

	bool invalid_user{false};
public:
	using hybrid_wrapper::hybrid_wrapper;
};

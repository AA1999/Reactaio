//
// Created by arshia on 11/6/23.
//

#pragma once
#include "../user_wrapper.h"
#include "ban_processor.h"


class guild_bans_wrapper: public user_wrapper {
	ban_vector bans;

	void wrapper_function() override;
	void check_permissions() override;

	void get_all_guild_bans(dpp::snowflake after = 1);
	void process_response();

	bool invalid_user{false};

public:
	using user_wrapper::user_wrapper;
};

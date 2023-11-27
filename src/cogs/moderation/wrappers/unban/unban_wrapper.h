//
// Created by arshia on 3/12/23.
//

#pragma once

#include "../../../base/datatypes/ban.h"
#include "../user_wrapper.h"
#include <vector>


class unban_wrapper: public user_wrapper {
	std::vector<reactaio::internal::ban> bans;

	void wrapper_function() override;
	void check_permissions() override;

	void get_all_guild_bans(dpp::snowflake after = 1);
	void process_unbans();
	void process_response();

public:
	using user_wrapper::user_wrapper;
};

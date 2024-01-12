//
// Created by arshia on 3/12/23.
//

#pragma once

#include "base_wrapper.h"

#include <dpp/dpp.h>
#include <vector>

class member_wrapper: public base_wrapper {
protected:
	std::vector<dpp::guild_member> members;
	std::vector<dpp::guild_member> members_with_errors;

	void check_permissions() override;
	void wrapper_function() override;

public:
	member_wrapper(const std::vector<dpp::guild_member>& members, moderation_command& command): members(members), base_wrapper(std::move(command)){}
	[[nodiscard]] bool are_all_errors() const override;
};

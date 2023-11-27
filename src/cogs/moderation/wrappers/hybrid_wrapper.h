//
// Created by arshia on 3/12/23.
//

#pragma once

#include "base_wrapper.h"

#include <dpp/dpp.h>
#include <variant>
#include <vector>

class hybrid_wrapper: public base_wrapper {
protected:
	std::vector<std::variant<dpp::user*, dpp::guild_member>> snowflakes;
	std::vector<dpp::user*> users_with_errors;
	std::vector<dpp::user*> users;
	std::vector<dpp::guild_member> members;

	void check_permissions() override;
	void wrapper_function() override = 0;

public:
	[[nodiscard]] bool are_all_errors() const override = 0;
	hybrid_wrapper(const std::vector<std::variant<dpp::user*, dpp::guild_member>>& snowflakes, moderation_command& command)
		: snowflakes(snowflakes), base_wrapper(std::move(command)) {}

};

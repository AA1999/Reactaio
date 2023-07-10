//
// Created by arshia on 3/12/23.
//

#pragma once

#include "base_wrapper.h"


class user_wrapper : public base_wrapper {
protected:
	std::vector<dpp::user*> users;
	std::vector<dpp::user*> users_with_errors;

	void check_permissions() override;
	void wrapper_function() override;
	[[nodiscard]] bool are_all_errors() const override;

public:
	user_wrapper(const std::vector<dpp::user*>& users, moderation_command& command): users(users), base_wrapper(std::move(command)){}
};

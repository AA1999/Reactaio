//
// Created by arshia on 2/16/23.
//

#pragma once

#include "../../base/datatypes/duration.h"
#include "../../base/helpers.h"
#include "../modules/moderation_command.h"

#include <dpp/dpp.h>
#include <optional>
#include <pqxx/pqxx>
#include <string_view>
#include <utility>
#include <vector>

class base_wrapper { // Base functor for all moderation punishments.

protected:
	std::optional<duration> duration;
	std::vector<std::string> errors{};
	std::vector<dpp::embed> embeds;
	moderation_command command;
	dpp::message error_message;
	bool are_errors{false};
	bool cancel_operation{false};

	virtual void check_permissions();
	virtual void wrapper_function() = 0;


public:
	virtual ~base_wrapper() = default;
	explicit base_wrapper(moderation_command command)
	    : command(std::move(command)), duration(parse_human_time(command.duration)) {}

	virtual void operator()() final;
	[[nodiscard]] bool is_error() const;
	[[nodiscard]] virtual bool are_all_errors() const = 0;
	[[nodiscard]] std::optional<dpp::message> error() const;
};

//
// Created by arshia on 2/15/23.
//

#pragma once

#include "../aliases.h"

#include <string>

/**
* @brief Wrapper struct for the constructor of a slash command.
*/
struct slash_command_properties {
	std::string name;
	std::string description;
	cluster_ptr bot;
};

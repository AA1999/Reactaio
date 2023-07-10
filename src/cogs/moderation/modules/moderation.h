//
// Created by arshia on 2/15/23.
//

#pragma once

#include "moderation_command.h"

#include <dpp/dpp.h>
#include <variant>
#include <vector>

namespace reactaio::moderation {
	void kick(const std::vector<dpp::guild_member>& members, const moderation_command& command);

}
//
// Created by arshia on 4/18/23.
//

#pragma once

#include "../../../base/datatypes/ban.h"

#include <vector>
#include <dpp/dpp.h>

using ban_vector = std::vector<reactaio::internal::ban>;


class ban_processor {

	dpp::guild* guild;
	ban_vector bans;


public:
	explicit ban_processor(dpp::guild* guild, dpp::ban_map& bans): guild(guild), bans() {
		for (const auto& [key, ban]: bans)
			this->bans.emplace_back(guild, ban);
	}

	[[nodiscard]] ban_vector guild_bans() const;


};

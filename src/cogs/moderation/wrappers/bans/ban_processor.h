//
// Created by arshia on 4/18/23.
//

#pragma once

#include "../../../base/datatypes/ban.h"

#include <vector>
#include <dpp/dpp.h>


using ban_vector = std::vector<reactaio::internal::ban>;

/**
 * @brief ban_processor - A wrapper for a dpp::ban which also includes the guild.
 */
class ban_processor {

	dpp::guild* guild;
	ban_vector bans;

public:
	explicit ban_processor(dpp::guild* guild, dpp::ban_map& bans): guild(guild), bans() {
		for(const auto& [key, ban]: bans)
			this->bans.emplace_back(guild, ban);
	}

	/**
	 * @brief guild_bans - Returns the bans in the wrapper
	 * @return The bans stored in the wrapper
	 */
	[[nodiscard]] ban_vector guild_bans() const;


};

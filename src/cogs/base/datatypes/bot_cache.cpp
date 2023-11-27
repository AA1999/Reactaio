//
// Created by arshia on 3/19/23.
//

#include "bot_cache.h"
#include "ban.h"

void bot_cache::insert_bans(dpp::guild* guild, const dpp::ban_map& ban_map) {
	for(auto const& [id, ban]: ban_map) {
		ban_cache.emplace(guild, ban);
	}
}

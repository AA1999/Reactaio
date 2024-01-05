//
// Created by arshia on 11/6/23.
//

#include <format>

#include "guild_bans_wrapper.h"
#include "ban_processor.h"
#include "../../../base/consts.h"


void guild_bans_wrapper::get_all_guild_bans(dpp::snowflake after) {
	command.bot->guild_get_bans(command.guild->id, 0, after, max_guild_ban_fetch, [this, after](const auto completion){
		if(completion.is_error()) {
			auto error = completion.get_error();
			errors.push_back(std::format("❌ Error {}: {}", error.code, error.message));
		}
		else {
			auto event_map = completion.template get<dpp::ban_map>();
			dpp::snowflake new_after{after};

			for(auto& [user_id, ban]: event_map) {
				if(new_after < user_id)
					new_after = user_id;
				bans.emplace_back(command.guild, ban);
			}

			if(event_map.size() < max_guild_ban_fetch) // All bans are fetched.
				return;
			get_all_guild_bans(new_after);
		}
	});
}


ban_vector guild_bans_wrapper::guild_bans() const {
	return bans;
}

std::vector<std::string> guild_bans_wrapper::what() const {
	return errors;
}

bool guild_bans_wrapper::is_error() const {
	return !errors.empty();
}

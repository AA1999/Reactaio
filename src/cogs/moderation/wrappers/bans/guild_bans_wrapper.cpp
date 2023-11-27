//
// Created by arshia on 11/6/23.
//

#include "guild_bans_wrapper.h"
#include "ban_processor.h"
#include "../../../base/consts.h"

void guild_bans_wrapper::wrapper_function() {
	user_wrapper::wrapper_function();
}

void guild_bans_wrapper::get_all_guild_bans(dpp::snowflake after) {
	command.bot->guild_get_bans(command.guild->id, 0, after, max_guild_ban_fetch, [this, after](const auto completion){
		if(completion.is_error()) {
			are_errors = true;
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


void guild_bans_wrapper::process_response() {
}


void guild_bans_wrapper::check_permissions() {
	user_wrapper::check_permissions();
}


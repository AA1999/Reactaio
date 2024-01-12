//
// Created by arshia on 3/19/23.
//

#pragma once

#include "ban.h"

#include <dpp/dpp.h>
#include <dpp/guild.h>
#include <set>



class bot_cache {
	std::set<reactaio::internal::ban> ban_cache;
public:
	bot_cache() = default;
	~bot_cache() = default;
	bot_cache(bot_cache& other) = default;
	bot_cache(bot_cache&& other) noexcept: ban_cache(std::move(other.ban_cache)){}

	void insert_bans(dpp::guild* guild, const dpp::ban_map& ban_map);
	void insert_ban(const dpp::snowflake& user_id);
	void clear_bans();
	void remove_ban(const dpp::snowflake& user_id);

	bool operator == (const bot_cache& other);

};

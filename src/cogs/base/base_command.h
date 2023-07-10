//
// Created by arshia on 2/15/23.
//

#pragma once

#include <dpp/dpp.h>
#include <optional>
#include <pqxx/pqxx>
#include <string_view>
#include <utility>

struct base_command {
	dpp::guild* guild;
	dpp::guild_member author;
	dpp::snowflake channel_id;
	dpp::cluster* bot;
	pqxx::connection* connection;
	std::optional<dpp::slashcommand_t> interaction; // Sometimes the command can be issued with automod etc
	base_command(dpp::cluster* bot, pqxx::connection* connection, dpp::guild* guild, dpp::guild_member  author,
				 const dpp::snowflake& channel_id, const std::optional<dpp::slashcommand_t>& interaction)
		: bot(bot), connection(connection), guild(guild), author(std::move(author)), channel_id(channel_id), interaction(interaction) {}
	
	base_command(base_command&& command) noexcept = default;
	base_command(base_command& command) = default;
 };
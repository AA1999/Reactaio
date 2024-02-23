//
// Created by arshia on 2/15/23.
//

#pragma once

#include <dpp/dpp.h>
#include <optional>
#include <pqxx/pqxx>
#include <string_view>

/**
 * @brief discord_command - The base wrapper for a called bot command that contains all the required information to pass into the wrapper.
 */
struct discord_command {
	dpp::guild* guild;
	dpp::guild_member author;
	dpp::snowflake channel_id;
	dpp::cluster* bot;
	pqxx::connection* connection;
	std::optional<dpp::slashcommand_t> interaction; // Sometimes the command can be issued with automod etc

	discord_command() = delete;

	/**
	 * @brief The main constructor
	 * @param bot The running bot instance.
	 * @param connection The current database connection.
	 * @param guild The guild the command is called from.
	 * @param author The command invoker.
	 * @param channel_id The channel command was invoked in.
	 * @param interaction The slash command interaction (if called from a slash command)
	 */
	discord_command(dpp::cluster* bot, pqxx::connection* connection, dpp::guild* guild, dpp::guild_member  author,
				 const dpp::snowflake& channel_id, const std::optional<dpp::slashcommand_t>& interaction) : guild(guild),
																											author(std::move(author)),
																											channel_id(channel_id),
																											bot(bot),
																											connection(connection),
																											interaction(interaction) {}

	discord_command(discord_command && command) noexcept = default;
	discord_command(discord_command & command) = default;
 };
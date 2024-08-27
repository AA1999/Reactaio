//
// Created by arshia on 2/15/23.
//

#pragma once

#include <dpp/dpp.h>
#include <optional>
#include <utility>

#include "../aliases.h"

/**
 * @brief discord_command - The base wrapper for a called bot command that contains all the required information to pass into the wrapper.
 */
struct discord_command {
	guild_ptr guild;
	member_ptr author;
	dpp::snowflake channel_id;
	cluster_ptr bot;
	connection_ptr connection;
	std::optional<interaction_ptr> interaction; // Sometimes the command can be issued with automod etc

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
	discord_command(cluster_ptr bot, connection_ptr connection, guild_ptr guild, member_ptr author,
				 const dpp::snowflake& channel_id, const std::optional<interaction_ptr>& interaction) : guild(std::move(guild)),
																											author(std::move(author)),
																											channel_id(channel_id),
																											bot(std::move(bot)),
																											connection(std::move(connection)),
																											interaction(interaction) {}

	discord_command(discord_command && command) noexcept = default;
	discord_command(discord_command & command) = delete;
 };
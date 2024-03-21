//
// Created by arshia on 2/15/23.
//

#pragma once

#include "../../core/aliases.h"
#include "../../core/discord_command.h"

#include <string_view>
#include <sys/types.h>
#include <utility>

struct moderation_command : public discord_command {
	std::string_view reason;
	std::string_view duration;
	ushort delete_message_days{0};
	bool appeal{false};

	moderation_command(std::shared_ptr<dpp::cluster> const& bot, std::shared_ptr<pqxx::connection> const& connection, std::shared_ptr<dpp::guild> const& guild, dpp::guild_member author, dpp::snowflake const& channel_id,
					   const std::optional<dpp::slashcommand_t>& interaction, std::string_view reason, std::string_view duration,
					   ushort const delete_message_days = 0, bool const appeal = false) : discord_command(bot, connection, guild, std::move(author), channel_id, interaction),
																			  reason(reason), duration(duration), delete_message_days(delete_message_days), appeal(appeal) {}

	moderation_command(moderation_command&& command) noexcept = default;
	moderation_command(moderation_command const& command): discord_command(command.bot, command.connection, command.guild, command.author, command.channel_id, command.interaction), reason(command.reason), duration(command.duration), delete_message_days(command.delete_message_days), appeal(command.appeal){}
};
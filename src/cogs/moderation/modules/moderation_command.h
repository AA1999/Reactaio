//
// Created by arshia on 2/15/23.
//

#pragma once

#include "../../base/aliases.h"
#include "../../base/base_command.h"

#include <string_view>
#include <sys/types.h>
#include <utility>

struct moderation_command : public base_command {
	std::string_view reason;
	std::string_view duration;
	ushort delete_message_days{0};
	bool appeal{false};

	moderation_command(dpp::cluster* bot, pqxx::connection* connection, dpp::guild* guild,
					   dpp::guild_member author, dpp::snowflake channel_id,
					   std::optional<dpp::slashcommand_t> interaction, std::string_view reason,
					   std::string_view duration, ushort delete_message_days = 0, bool appeal = false)
		: base_command(bot, connection, guild, std::move(author), channel_id, std::move(interaction)),
		  reason(reason), duration(duration), delete_message_days(delete_message_days), appeal(appeal) {}

	moderation_command(moderation_command&& command) noexcept = default;
};
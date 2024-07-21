//
// Created by arshia on 2/15/23.
//

#pragma once

#include "../../core/discord_command.h"
#include "../../core/strings.h"

#include <string_view>

struct moderation_command : public discord_command {
	std::string reason;
	std::string_view duration;
	ushort delete_message_days{0};
	bool appeal{false};

	moderation_command(std::shared_ptr<dpp::cluster> const& bot, connection_ptr connection, std::shared_ptr<dpp::guild> const& guild, const member_ptr& author, dpp::snowflake const& channel_id,
					   const std::optional<interaction_ptr>& interaction, std::string_view reason, std::string_view duration,
					   ushort const delete_message_days = 0, bool const appeal = false) : discord_command(bot, std::move(connection), guild, author, channel_id, interaction),
					   reason(reason), duration(duration), delete_message_days(delete_message_days), appeal(appeal)
	{
		auto const everyone_role = std::format("<!@{}>", std::to_string(guild->id));
		replace_all(this->reason, everyone_role, ""); // Exploit regarding @everyone mentions in bot commands. I accidentally noticed this after the bun server bot had a similar exploit.
	}

	moderation_command(moderation_command&& command) noexcept = default;
	moderation_command(const moderation_command& command) = delete;
};
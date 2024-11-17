//
// Created by arshia on 2/15/23.
//

#pragma once

#include "aliases.h"

extern inline ullong constexpr bot_max_embed_chars = 4096;
extern inline ullong constexpr bot_max_msg_chars = 4000;
extern inline ullong constexpr max_timeout_seconds = 2419200;
extern inline std::uint8_t constexpr bot_max_embeds = 10;
extern inline std::uint8_t constexpr max_global_commands = 100;
extern inline std::uint8_t constexpr max_guild_commands = 200;
extern inline ushort constexpr max_guild_ban_fetch = 1000;
extern inline std::uint8_t constexpr  max_guild_audit_log_fetch = 100;
extern inline std::uint8_t constexpr max_timeout_days = 28;
extern inline std::string_view constexpr iso_format{"%Y-%m-%d %H:%M:%S"};
extern inline std::uint32_t constexpr max_ban_remove_seconds = 604800;
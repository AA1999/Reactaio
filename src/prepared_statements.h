//
// Created by arshia on 3/1/24.
//

#pragma once

#include "cogs/core/datatypes/fixed_map.h"

#include <cstdint>

constexpr std::uint8_t MAP_SIZE{30};

namespace reactaio::internal {
	extern inline const fixed_map<std::string, std::string, MAP_SIZE> prepated_statements {
		{"kick_modlog", "SELECT member_kick, modlog, public_modlog FROM config WHERE guild_id = $1"},
		{"casecount", "SELECT case_id FROM modcase WHERE guild_id = $1 ORDER BY case_id DESC LIMIT 1"},
		{"ban_modlog", "SELECT member_ban_add, modlog, public_modlog FROM config WHERE guild_id = $1"},
		{"modlog", "SELECT modlog, public_modlog FROM config WHERE guild_id = $1"},
		{"botlog", "SELECT bot_error_logs FROM config WHERE guild_id = $1"},
		{"modcase_insert", "INSERT INTO modcase(guild_id, case_id, action, mod_id, punished_id, reason) VALUES($1, $2, $3, $4, $5, $6)"},
		{"modcase_insert_duration", "INSERT INTO modcase(guild_id, case_id, action, duration, mod_id, punished_id, reason) VALUES($1, $2, $3, $4, $5, $6, $7)"},
		{"modcase_view", "SELECT action, mod_id, punished_id, reason, duration FROM modcase WHERE case_id = $1 AND guild_id = $2"},
		{"modcase_update_reason", "UPDATE modcase SET reason = $1 WHERE case_id = $1 AND guild_id = $2 RETURNING case_id"},
		{"command_insert", "INSERT INTO command_logs(guild_id, author_id, name, issues_at) VALUES($1, $2, $3, $4)"},
		{"get_ban_remove_days", "SELECT ban_remove_days FROM config WHERE guild_id = $1"},
		{"get_ban_id", "SELECT ban_id FROM tempbans WHERE guild_id = $1 ORDER BY ban_id DESC LIMIT 1"},
		{"tempban", "INSERT INTO tempbans(ban_id, user_id, guild_id, mod_id, start_date, end_date, reason) VALUES($1, $2, $3, $4, $5, $6, $7) ON CONFLICT ON CONSTRAINT tempbans_user_id_guild_id_key  DO UPDATE SET start_date = $5, end_date = $6, reason = $7"},
		{"get_unban_log", "SELECT member_ban_remove FROM config WHERE guild_id = $1"},
		{"get_mute_id", "SELECT mute_id FROM tempmutes WHERE guild_id = $1 ORDER BY mute_id DESC LIMIT 1"},
		{"get_mute_role", "SELECT mute_role FROM config WHERE guild_id = $1"},
		{"tempmute", "INSERT INTO tempmutes(mute_id, user_id, guild_id, mod_id, start_date, end_date, reason) VALUES($1, $2, $3, $4, $5, $6, $7) ON CONFLICT ON CONSTRAINT tempmutes_user_id_guild_id_key DO UPDATE SET start_date = $5, end_date = $6, reason= $7"},
		{"view_tempmute", "SELECT mute_id, user_id, mod_id, start_date, end_date, reason FROM tempmutes WHERE user_id = $1 AND guild_id = $2"},
		{"protected_roles", "SELECT protected_roles FROM config WHERE guild_id = $1"},
		{"check_timeout", "SELECT use_timeout FROM config WHERE guild_id = $1"},
		{"get_timeout_id", "SELECT timeout_id FROM permanent_timeouts WHERE guild_id = $1 ORDER BY timeout_id DESC LIMIT 1"},
		{"permanent_timeout", "INSERT INTO permanent_timeouts(timeout_id, user_id, guild_id, author_id, reason) VALUES($1, $2, $3, $4, $5) ON CONFLICT ON CONSTRAINT permanent_timeouts_pkey DO UPDATE SET REASON = $5"},
		{"hardban_get", "SELECT user_id FROM hardbans WHERE guild_id = $1"},
		{"view_warnings", "SELECT warn_id, reason FROM warnings WHERE guild_id = $1 AND user_id = $2"},
		{"view_guild_warnings", "SELECT warn_id, user_id, mod_id, reason FROM warnings WHERE guild_id = $1"},
		{"view_guild_muted_members", "SELECT case_id, action, mod_id, punished_id, action, duration, reason FROM modcase WHERE guild_id = $1 AND (action = 'Mute' OR action = 'Timeout')"},
		{"warning_lookup", "SELECT user_id FROM warnings WHERE warn_id = $1 AND guild_id = $2"},
		{"remove_warning", "DELETE FROM warnings WHERE warn_id = $1 AND guild_id = $2 RETURNING warn_id"},
		{"clear_warnings", "DELETE FROM warnings WHERE user_id = $1 AND guild_id = $2 RETURNING user_id"},
		{"clear_guild_warnings", "DELETE FROM warnings WHERE guild_id = $1 RETURNING guild_id"}
	};
}
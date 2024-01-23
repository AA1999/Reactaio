//
// Created by arshia on 1/3/24.
//

#include "logging.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> log_startup(std::string_view log_filename) {
	ushort const EIGHT_MEGABYTES = 8192;
	uint const FIVE_GIGABYTES = 1024 * 1024 * 5;
	spdlog::init_thread_pool(EIGHT_MEGABYTES, 2);
	std::vector<spdlog::sink_ptr> sinks;
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_filename.data(), FIVE_GIGABYTES, 10);
	sinks.push_back(stdout_sink);
	sinks.push_back(rotating);
	auto logger = std::make_shared<spdlog::async_logger>("logs", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	spdlog::register_logger(logger);
	logger->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] [th#%t]%$ : %v");
	logger->set_level(spdlog::level::level_enum::debug);
	return logger;
}

void logger([[maybe_unused]] dpp::cluster* bot, const dpp::log_t& event,
			const std::shared_ptr<spdlog::logger>& log) {
	switch (event.severity) {
		case dpp::ll_trace:
			log->trace("{}", event.message);
			break;
		case dpp::ll_debug:
			log->debug("{}", event.message);
			break;
		case dpp::ll_info:
			log->info("{}", event.message);
			break;
		case dpp::ll_warning:
			log->warn("{}", event.message);
			break;
		case dpp::ll_error:
			log->error("{}", event.message);
			break;
		case dpp::ll_critical:
		default:
			log->critical("{}", event.message);
			break;
	}
}

void create_prepared_statements(pqxx::connection *connection) {
	connection->prepare("kick_modlog", "SELECT member_kick, modlog, public_modlog FROM config WHERE guild_id = $1");
	connection->prepare("casecount", "SELECT case_id FROM modcase WHERE guild_id = $1 ORDER BY case_id DESC LIMIT 1");
	connection->prepare("ban_modlog", "SELECT member_ban_add, modlog, public_modlog FROM config WHERE guild_id = $1");
	connection->prepare("modlog", "SELECT modlog, public_modlog FROM config WHERE guild_id = $1");
	connection->prepare("botlog", "SELECT bot_error_logs FROM config WHERE guild_id = $1");
	connection->prepare("modcase_insert", "INSERT INTO modcase(guild_id, case_id, action, mod_id, punished_id, reason)"
										  "VALUES($1, $2, $3, $4, $5, $6)");
	connection->prepare("modcase_insert_duration", "INSERT INTO modcase(guild_id, case_id, action, duration, mod_id, "
												   "punished_id, reason)"
												   "VALUES($1, $2, $3, $4, $5, $6, $7)");
	connection->prepare("modcase_view", "SELECT action, mod_id, punished_id, reason, duration FROM modcase WHERE case_id = $1 AND guild_id = $2");
	connection->prepare("modcase_update_reason", "UPDATE modcase SET reason = $1 WHERE case_id = $1 AND guild_id = $2 RETURNING case_id");
	connection->prepare("command_insert", "INSERT INTO command_logs(guild_id, author_id, name, issues_at) VALUES($1, $2, $3,"
										  " $4)");
	connection->prepare("get_ban_remove_days", "SELECT ban_remove_days FROM config WHERE guild_id = $1");
	connection->prepare("get_ban_id", "SELECT ban_id FROM tempbans WHERE guild_id = $1 ORDER BY ban_id DESC LIMIT 1");
	connection->prepare("tempban", "INSERT INTO tempbans(ban_id, user_id, guild_id, mod_id, start_date, end_date, reason) VALUES($1,"
								   " $2, $3, $4, $5, $6, $7) ON CONFLICT ON CONSTRAINT tempbans_user_id_guild_id_key  DO UPDATE SET start_date = $5, end_date = $6, reason = $7"
								   ")");
	connection->prepare("get_unban_log", "SELECT member_ban_remove FROM config WHERE guild_id = $1");
	connection->prepare("get_mute_id", "SELECT mute_id FROM tempmutes WHERE guild_id = $1 ORDER BY mute_id DESC LIMIT 1");
	connection->prepare("get_mute_role", "SELECT mute_role FROM config WHERE guild_id = $1");
	connection->prepare("tempmute", "INSERT INTO tempmutes(mute_id, user_id, guild_id, mod_id, start_date, end_date, reason) VALUES"
									"($1, $2, $3, $4, $5, $6, $7) ON CONFLICT ON CONSTRAINT tempmutes_user_id_guild_id_key DO UPDATE SET start_date = $5, end_date = $6, reason= $7");
	connection->prepare("protected_roles", "SELECT protected_roles FROM config WHERE guild_id = $1");
	connection->prepare("check_timeout", "SELECT use_timeout FROM config WHERE guild_id = $1");
	connection->prepare("get_timeout_id", "SELECT timeout_id FROM permanent_timeouts WHERE guild_id = $1 ORDER BY timeout_id DESC LIMIT 1");
	connection->prepare("permanent_timeout", "INSERT INTO permanent_timeouts(timeout_id, user_id, guild_id, author_id, reason) VALUES($1, $2, $3, $4, $5) ON CONFLICT ON CONSTRAINT permanent_timeouts_pkey DO UPDATE SET REASON = $5");
	connection->prepare("hardban_get", "SELECT user_id FROM hardbans WHERE guild_id = $1");
	connection->prepare("view_warnings", "SELECT warn_id, reason FROM warnings WHERE guild_id = $1 AND user_id = $2");
	connection->prepare("warning_lookup", "SELECT user_id FROM warnings WHERE warn_id = $1 AND guild_id = $2");
	connection->prepare("remove_warning", "DELETE FROM warnings WHERE warn_id = $1 AND guild_id = $2 RETURNING warn_id");
	connection->prepare("clear_warnings", "DELETE FROM warnings WHERE user_id = $1 AND guild_id = $2 RETURNING user_id");
	connection->prepare("clear_guild_warnings", "DELETE FROM warnings WHERE guild_id = $1 RETURNING guild_id");
}


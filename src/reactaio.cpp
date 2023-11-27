#include <dpp/dpp.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string_view>

void logger([[maybe_unused]] dpp::cluster* bot, const dpp::log_t& event,
			const std::shared_ptr<spdlog::logger>& log);

void create_prepared_statements(pqxx::connection *connection);

int main() {

	auto file = std::ifstream("~/.reactaio/config.json");

	using json = nlohmann::json;

	json data  = json::parse(file);
	auto TOKEN = to_string(data.at("TOKEN"));

	file.close();

	dpp::cluster bot(TOKEN, dpp::intents::i_all_intents);

	// Set up pqxx::connection

	auto db_config_file = std::ifstream {"~/.reacatio/database.json"};

	json db_config = json::parse(db_config_file);
	db_config_file.close();

	std::string connection_config = fmt::format("host={} port={} dbname={} user={} password='{}'", db_config.at("host"),
												db_config.at("port"), db_config.at("dbname"), db_config.at("user"),
												db_config.at("password"));

	pqxx::connection connection{connection_config};

	std::string_view log_name;
	log_name = fmt::format("logs/{}-reactaio.log", dpp::utility::current_date_time());

	// PostgreSQL prepared statements

	create_prepared_statements(&connection);

	// Set up spdlog logger

	spdlog::init_thread_pool(8192, 2);
	std::vector<spdlog::sink_ptr> sinks;
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_name.data(), 1024 * 1024 * 5, 10);
	sinks.push_back(stdout_sink);
	sinks.push_back(rotating);
	std::shared_ptr<spdlog::logger> log;
	log = std::make_shared<spdlog::async_logger>("logs", sinks.begin(), sinks.end(), spdlog::thread_pool(),
												 spdlog::async_overflow_policy::block);
	spdlog::register_logger(log);
	log->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] [th#%t]%$ : %v");
	log->set_level(spdlog::level::level_enum::debug);

	log->info(fmt::format("Successfully connected to database {} at port {}.", connection.dbname(), connection.port()));

	bot.on_ready([&bot, &log]([[maybe_unused]] const dpp::ready_t& event) {
		log->info(fmt::format("{}({}) is ready!", bot.me.format_username(), bot.me.id));
	});

	// Integrate spdlog logger to D++ log events
	bot.on_log([&bot, log](auto&& PH1) {
		return logger(&bot, std::forward<decltype(PH1)>(PH1), log);
	});

	std::unordered_map<std::string, dpp::slashcommand> global_commands;

	bot.start(dpp::st_wait);
	return 0;
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
	connection->prepare("command_insert", "INSERT INTO command_logs(guild_id, author_id, name, issues_at) VALUES($1, $2, $3,"
								   " $4)");
	connection->prepare("get_ban_remove_days", "SELECT ban_remove_days FROM config WHERE guild_id = $1");
	connection->prepare("get_ban_id", "SELECT ban_id FROM tempbans WHERE guild_id = $1 ORDER BY mute_id DESC LIMIT 1");
	connection->prepare("tempban", "INSERT INTO tempbans(ban_id, user_id, guild_id, mod_id, start_date, end_date, reason) VALUES($1,"
							" $2, $3, $4, $5, $6, $7) ON CONFLICT ON CONSTRAINT tempbans_user_id_guild_id_key  DO UPDATE SET start_date = $5, end_date = $6, reason = $7"
							")");
	connection->prepare("get_mute_id", "SELECT mute_id FROM tempmutes WHERE guild_id = $1 ORDER BY mute_id DESC LIMIT 1");
	connection->prepare("get_mute_role", "SELECT mute_role FROM config WHERE guild_id = $1");
	connection->prepare("tempmute", "INSERT INTO tempmutes(mute_id, user_id, guild_id, mod_id, start_date, end_date, reason) VALUES"
							 "($1, $2, $3, $4, $5, $6, $7) ON CONFLICT ON CONSTRAINT tempmutes_user_id_guild_id_key DO UPDATE SET start_date = $5, end_date = $6, reason= $7");
	connection->prepare("protected_roles", "SELECT protected_roles FROM config WHERE guild_id = $1");
	connection->prepare("check_timeout", "SELECT use_timeout FROM config WHERE guild_id = $1");
	connection->prepare("get_timeout_id", "SELECT timeout_id FROM permanent_timeouts WHERE guild_id = $1 ORDER BY timeout_id DESC LIMIT 1");
	connection->prepare("permanent_timeout", "INSERT INTO permanent_timeouts(timeout_id, user_id, guild_id, author_id, reason) VALUES($1, $2, $3, $4, $5) ON CONFLICT ON CONSTRAINT permanent_timeouts_pkey DO UPDATE SET REASON = $5");
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
};
#include <dpp/dpp.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <format>

#include "logging.h"

int main() {
	auto file = std::ifstream{"~/.reactaio/config.json"};

	using json = nlohmann::json;

	json data  = json::parse(file);
	auto TOKEN = to_string(data.at("TOKEN"));

	file.close();

	dpp::cluster bot(TOKEN, dpp::intents::i_all_intents);

	// Set up pqxx::connection

	auto db_config_file = std::ifstream {"~/.reacatio/database.json"};

	json db_config = json::parse(db_config_file);
	db_config_file.close();

	std::string connection_config = std::format("host={} port={} dbname={} user={} password='{}'", to_string(db_config.at("host")),
												to_string(db_config.at("port")), to_string(db_config.at("dbname")), to_string(db_config.at("user")),
												to_string(db_config.at("password")));

	pqxx::connection connection{connection_config};


	// PostgreSQL prepared statements

	create_prepared_statements(&connection);

	// Set up spdlog logger

	std::string log_name = std::format("logs/{}-reactaio.log", dpp::utility::current_date_time());

	auto log = log_startup(log_name);

	log->info(std::format("Successfully connected to database {} at port {}.", connection.dbname(), connection.port()));

	bot.on_ready([&bot, &log]([[maybe_unused]] const dpp::ready_t& event) {
		log->info(std::format("{}({}) is ready!", bot.me.format_username(), bot.me.id.str()));
	});

	// Integrate spdlog logger to D++ log events
	bot.on_log([&bot, log](auto&& PH1) {
		return logger(&bot, std::forward<decltype(PH1)>(PH1), log);
	});

	std::unordered_map<std::string, dpp::slashcommand> global_commands;

	bot.start(dpp::st_wait);
	return 0;
}
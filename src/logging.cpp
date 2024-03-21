//
// Created by arshia on 1/3/24.
//

#include "logging.h"
#include "prepared_statements.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> log_startup(std::string_view log_filename) {
	ushort constexpr EIGHT_MEGABYTES = 8192;
	uint constexpr FIVE_GIGABYTES = 1024 * 1024 * 5;
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

void create_prepared_statements(std::shared_ptr<pqxx::connection> connection) {
	std::ranges::for_each(reactaio::internal::prepated_statements, [&](auto const& pair) {
		auto const& [key, value] = pair;
		connection->prepare(key, value);
	});
}


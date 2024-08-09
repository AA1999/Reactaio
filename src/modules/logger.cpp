//
// Created by arshia on 7/17/24.
//

#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace reactaio {
	void logger::innit() {
		ushort constexpr EIGHT_MEGABYTES = 8192;
		uint constexpr FIVE_GIGABYTES = 1024 * 1024 * 5;
		spdlog::init_thread_pool(EIGHT_MEGABYTES, 2);
		std::vector<spdlog::sink_ptr> sinks;
		std::string const log_file_name = std::format("logs/reactaio-{:%Y-%m-%d %H:%M:%S}.log", std::chrono::system_clock::now());
		auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, FIVE_GIGABYTES, 10);
		sinks.push_back(stdout_sink);
		sinks.push_back(rotating);
		auto logger = std::make_shared<spdlog::async_logger>("logs", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
		spdlog::register_logger(logger);
		logger->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] [th#%t]%$ : %v");
		logger->set_level(spdlog::level::level_enum::debug);
	}

	void logger::start() {} // No specific impl

	void logger::stop() {} // No specific impl

	void logger::info(const std::string_view message) const {
		m_logger->info(message);
	}

	void logger::trace(const std::string_view message) const {
		m_logger->trace(message);
	}

	void logger::debug(const std::string_view message) const {
		m_logger->debug(message);
	}

	void logger::warn(const std::string_view message) const {
		m_logger->warn(message);
	}

	void logger::error(const std::string_view message) const {
		m_logger->error(message);
	}

	void logger::critical(const std::string_view message) const {
		m_logger->critical(message);
	}
}// namespace reactaio

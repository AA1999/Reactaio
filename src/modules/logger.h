//
// Created by arshia on 7/17/24.
//

#pragma once
#include "module.h"

#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace reactaio {
	/**
	 * @brief Logging module
	 */
	class logger final: public module {
		std::shared_ptr<spdlog::async_logger> m_logger;
	public:
		logger(): module("logger", dependency_t{}){};
		~logger() override = default;

		/**
		 * @brief Initializes the moderation module with the provided modules.
		 * @param modules A reference of a map of modules.
		 */
		void innit(const module_map& modules) override;

		/**
		 * @brief Starts the module.
		 */
		void start() override;

		/**
		 * @brief Stops the module.
		 */
		void stop() override;

		/**
		 * @brief Provides an information log.
		 * @param message The message to log.
		 */
		void info(std::string_view message) const;

		/**
		 * @brief Provides a trace log.
		 * @param message The message to log.
		 */
		void trace(std::string_view message) const;

		/**
		 * @brief Provides a debug log.
		 * @param message The message to log.
		 */
		void debug(std::string_view message) const;

		/**
		 * @brief Provides a warning log.
		 * @param message The message to log.
		 */
		void warn(std::string_view message) const;

		/**
		 * @brief Provides an error log.
		 * @param message The message to log.
		 */
		void error(std::string_view message) const;

		/**
		 * @brief Provides a critical log.
		 * @param message The message to log.
		 */
		void critical(std::string_view message) const;
	};
}
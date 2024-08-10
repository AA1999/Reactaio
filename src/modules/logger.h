//
// Created by arshia on 7/17/24.
//

#pragma once
#include "module.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace reactaio {
	class logger final : public module {
		std::shared_ptr<spdlog::async_logger> m_logger;
	public:
		logger() = default;
		~logger() override = default;

		[[nodiscard]] constexpr std::string_view name() const noexcept override {
			return "logger";
		}

		void innit() override;
		void start() override;
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
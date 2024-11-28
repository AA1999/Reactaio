//
// Created by arshia on 7/17/24.
//

#pragma once

#include "module.h"

#include <dpp/dpp.h>
#include <spdlog/async.h>

namespace reactaio {
	class logger final : public module {
		std::shared_ptr<spdlog::async_logger> m_logger;
		bool m_silent{false};
		bool m_is_running{false};
		bool m_is_initialized{false};
	public:
		logger() = default;

		/**
		 * @brief Constructor for cases where the logger needs to be silent.
		 * @param silent Should the logger be silent on init? (Used for internal loggers)
		 */
		explicit logger(const bool silent): m_silent(silent) {}

		~logger() override = default;

		/**
		 * @brief What's the module name?
		 * @return The name of the module.
		 */
		[[nodiscard]] constexpr std::string name() const noexcept override {
			return "logger";
		}

		/**
		 * @brief Initializes the module.
		 */
		void init() override;

		/**
		 * @brief Starts the module.
		 */
		void start() override;

		/**
		 * @brief Stops the module.
		 */
		void stop() override;

		/**
		 * @brief Is the bot running?
		 * @return true if the bot is running.
		 * @return false if the bot isn't running/stopped.
		 * @note This is an abstract function.
		 */
		[[nodiscard]] constexpr bool is_running() const noexcept override {
			return m_is_running;
		}

		/**
		 * @brief Is the bot initialized?
		 * @return true if the bot is initialized.
		 * @return false if the bot isn't initialized.
		 * @note This is an abstract function.
		 */
		[[nodiscard]] constexpr bool is_initialized() const noexcept override {
			return m_is_initialized;
		}

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

		/**
		 * @brief Automatically logs an event that was logged by dpp itself.
		 * @param event The DPP event log.
		 */
		void log_event(const dpp::log_t& event) const;
	};
}
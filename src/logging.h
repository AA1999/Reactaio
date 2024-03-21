//
// Created by arshia on 1/3/24.
//

#pragma once


#include <spdlog/spdlog.h>
#include <dpp/dpp.h>
#include <string_view>
#include <memory>
#include <pqxx/pqxx>

/**
 * @brief log_startup - Prepares the logging process
 * @param log_filename The filename to store logfiles in. It should be a std::format pattern.
 * @return A shared pointer to the logger object.
 */
std::shared_ptr<spdlog::logger> log_startup(std::string_view log_filename);


/**
 * @brief logger - Sets up logger event loop.
 * @param bot The bot to call the logger on.
 * @param event The logging event
 * @param log The logger object.
 */
void logger([[maybe_unused]] dpp::cluster* bot, const dpp::log_t& event,
			const std::shared_ptr<spdlog::logger>& log);

/**
 * @brief create_prepared_statements - Creates prepared statements for the SQL connection.
 * @param connection The active Postgresql connection.
 */
void create_prepared_statements(std::shared_ptr<pqxx::connection> connection);
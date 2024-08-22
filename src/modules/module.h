//
// Created by arshia on 7/16/24.
//

#pragma once

#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace reactaio {
	struct module {
		using module_ptr = std::unique_ptr<module>;
		using module_map = std::unordered_map<std::string, module_ptr>;

		/**
		 * @brief Module name.
		 * @return Name of the module.
		 */
		[[nodiscard]] virtual constexpr std::string name() const = 0;

		/**
		 * @brief Module dependencies.
		 * @return The dependencies of the module.
		 */
		[[nodiscard]] virtual module_map dependencies() const;

		/**
		 * @brief Get all submodules of this module.
		 * @return A map of all submodules by name.
		 */
		[[nodiscard]] virtual module_map submodules() const;


		/**
		 * @brief Is the bot running?
		 * @return true if the bot is running.
		 * @return false if the bot isn't running/stopped.
		 * @note This is an abstract function.
		 */
		[[nodiscard]] virtual constexpr bool is_running() const = 0;

		/**
		 * @brief Is the bot initialized?
		 * @return true if the bot is initialized.
		 * @return false if the bot isn't initialized.
		 * @note This is an abstract function.
		 */
		[[nodiscard]] virtual constexpr bool is_initialized() const = 0;
	
		virtual ~module() = default;
		module() = default;
		module(const module&) = delete;
		module& operator=(const module&) = delete;
		module(module&&) = default;

		/**
		 * @brief Initializes the module.
		 * @note This is an abstract function.
		 */
		virtual void innit() = 0;

		/**
		 * @brief Starts the module.
		 * @note This is an abstract function.
		 */
		virtual void start() = 0;

		/**
		 * @brief Stops the module.
		 * @note This is an abstract function.
		 */
		virtual void stop() = 0;


		/**
		 * @brief Reloads the module.
		 */
		void reload();
	};

}
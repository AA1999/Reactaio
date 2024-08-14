//
// Created by arshia on 7/16/24.
//

#pragma once

#include <memory>
#include <span>
#include <unordered_map>

namespace reactaio {
	struct module {
		using dependency_t = std::span<std::string_view>;
		using module_ptr = std::unique_ptr<module>;
		using submodules_t = std::unordered_map<std::string_view, module_ptr>;

		/**
		 * @brief Module name.
		 * @return Name of the module.
		 */
		[[nodiscard]] virtual constexpr std::string_view name() const = 0;

		/**
		 * @brief Module dependencies.
		 * @return The dependencies of the module.
		 */
		[[nodiscard]] virtual dependency_t dependencies() const {
			return {};
		};

		/**
		 * @brief Get all submodules of this module.
		 * @return A map of all submodules by name.
		 */
		[[nodiscard]] virtual submodules_t submodules() const;
	
		virtual ~module() = default;
		module() = default;
		module(const module&) = default;
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
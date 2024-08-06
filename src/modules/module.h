//
// Created by arshia on 7/16/24.
//

#pragma once

#include "../cogs/core/containers/unique_vector.h"

#include <memory>
#include <span>
#include <unordered_map>

namespace reactaio {
	struct module {

		using dependency_t = std::span<std::string_view>;
		using module_ptr = std::unique_ptr<module>;

		[[nodiscard]] virtual constexpr std::string_view name() const = 0;

		[[nodiscard]] virtual dependency_t dependencies() const {
			return {};
		};
	
		virtual ~module() = default;
		module(const module&) = delete;
		module& operator=(const module&) = delete;

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

	};

}
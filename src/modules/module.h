//
// Created by arshia on 7/16/24.
//

#pragma once

#include "../cogs/core/datatypes/unique_vector.h"

#include <memory>
#include <unordered_map>

namespace reactaio {
	using module_ptr = std::shared_ptr<class module>;
	using module_map = std::unordered_map<std::string, module_ptr>;
	using dependency_t = internal::unique_vector<std::string>;

	class module {
	protected:
		std::string m_name;
		dependency_t m_dependencies;
	public:
		module() = delete;
		module(std::string name, dependency_t dependencies): m_name(std::move(name)), m_dependencies(std::move(dependencies)) {};
		virtual ~module() = default;

		[[nodiscard]] constexpr std::string name() const {
			return m_name;
		}

		[[nodiscard]] dependency_t dependencies() const {
			return m_dependencies;
		}

		/**
		 * @brief Starts all the provided modules.
		 * @param modules Map of all modules managed by application.
		 * @note This is an abstract method.
		 */
		virtual void innit(const module_map& modules) = 0;

		/**
		 * @brief Stops the module.
		 * @note This is an abstract method.
		 */
		virtual void stop() = 0;

		/**
		 * @brief Starts the module.
		 * @note This is an abstract method.
		 */
		virtual void start() = 0;

	};

}
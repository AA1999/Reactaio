 //
// Created by arshia on 7/18/24.
//

#pragma once

#include "FileWatch.hpp"
#include "logger.h"
#include "module.h"


#include <filesystem>
#include <shared_mutex>
#include <string_view>
#include <utility>

namespace reactaio {
	using modules_manager_ptr = std::shared_ptr<class module_manager>;

	class module_manager final: public module, public std::enable_shared_from_this<module> {

		class internal_module;
		using internal_modules = std::unordered_map<std::string, internal_module>;

		class internal_module {
			void* m_library_handler;

			module_ptr m_module;
			dependency_t m_deps;

			std::string_view m_file_path;

			bool m_is_initialized{false};
			bool m_is_running{false};
		public:
			~internal_module();

			/**
			 * @param library_handler The librady handler using dlopen
			 * @param module Pointer to the underlying module.
			 * @param file_path Path to the filename of module.
			 * @param deps List of dependencies.
			 */
			internal_module(void* library_handler, module_ptr module, const std::string_view file_path, dependency_t deps): m_library_handler(library_handler), m_module(std::move(module)), m_deps(std::move(deps)), m_file_path(file_path){};
			internal_module() = delete;

			/**
			 * @brief Get the underlying library handler using dlopen.
			 * @return A pointer to the underlying library handler.
			 */
			[[nodiscard]] constexpr void* library_handler() const {
				return m_library_handler;
			}

			/**
			 * @brief Get the underlying module.
			 * @return A pointer to the underlying module.
			 */
			[[nodiscard]] module_ptr module() const {
				return m_module;
			}

			/**
			 * @brief Get the module file path.
			 * @return Path to the module file.
			 */
			[[nodiscard]] constexpr std::string_view file_path() const {
				return m_file_path;
			}

			/**
			 * @brief Is the module initialized?
			 * @return true if the internal module is initialized.
			 * @return false if the internal module isn't initialized.
			 */
			[[nodiscard]] constexpr bool is_initialized() const {
				return m_is_initialized;
			}

			/**
			 * @brief Is the module running?
			 * @return true if the internal module is running.
			 * @return false if the internal module is stopped.
			 */
			[[nodiscard]] constexpr bool is_running() const {
				return m_is_running;
			}

			/**
			 * @brief Get module dependencies.
			 * @return The module dependencies.
			 */
			[[nodiscard]] dependency_t dependencies() const {
				return m_deps;
			}

			/**
			 * 
			 * @return 
			 */
			[[nodiscard]] constexpr std::string_view get_file_path() const {
				return m_file_path;
			}

			/**
			 * @brief Are all dependencies loaded?
			 * @param modules Modules passed to the internal module.
			 * @return true if the modules match the dependencies of the internal module.
			 * @return false if the modules don't match the dependencies of the internal module.
			 */
			[[nodiscard]] bool has_sufficient_dependencies(const internal_modules& modules) const;

			/**
			 * @brief Initialzes the internal module.
			 * @param modules Modules passed to the internal module.
			 */
			void innit(const internal_modules& modules);

			/**
			 * @brief Adds a dependency to the internal module.
			 * @param name Name of the dependency.
			 */
			void add_dependency(std::string_view &name);

			/**
			 * @brief Removed a dependency from the internal module.
			 * @param name Name of the dependency.
			 */
			void remove_dependency(std::string_view &name);

			/**
			 * @brief Starts the internal module.
			 */
			void start();

			/**
			 * @brief Stops the internal module.
			 */
			void stop();
		};

		std::shared_mutex m_mutex;

		filewatch::FileWatch<std::string> m_filewatch;

		internal_modules m_internal_modules;

		std::filesystem::path m_path;

		logger m_logger{};

		bool m_allow_modules_load{true};

		internal::unique_vector<internal_module> m_load_order;

		explicit module_manager(const std::filesystem::path& modules_path);

		/**
		 * @brief Internal module stop util.
		 * @param name Name of the internal module to sotp.
		 */
		void stop_internal_module(std::string_view& name);

		/**
		 * @brief Internal module start util.
		 * @param name Name of the internal module to start.
		 */
		void start_internal_module(std::string_view& name);

		/**
		 * @brief Internal module innit util.
		 * @param name Name of the internal module to initialize.
		 */
		void innit_internal_module(std::string_view& name);

		/**
		 * @brief Internal module load util.
		 * @param name Name of the internal module to load.
		 * @return Name of the loaded module.
		 */
		std::string_view load_internal_module(std::string_view &name);

	public:
		/**
		 * @brief Creates an instance of module manager using the given path.
		 * @param modules_path Path to save the module in.
		 * @return An instance of this class.
		 */
		static modules_manager_ptr create(const std::filesystem::path& modules_path);

		~module_manager() override = default;

		/**
		 * @brief Initialize the module manager.
		 * @param modules Modules to initialize the module manager with.
		 */
		void innit(const module_map& modules) override;

		/**
		 * @brief Starts the module manager.
		 */
		void start() override;

		/**
		 * @brief Stops the module manager.
		 */
		void stop() override;

		/**
		 * @brief Initializes the internal modules.
		 */
		void innit_modules();

		/**
		 * @brief Initialize the given module.
		 * @param module_name Module name to initialize.
		 */
		void innit_module(std::string_view& module_name);

		/**
		 * @brief Start the given module.
		 * @param module_name Module name to start.
		 */
		void start_module(std::string_view& module_name);

		/**
		 * @brief Stop the given module.
		 * @param module_name Module name to stop.
		 */
		void stop_module(std::string_view& module_name);

		/**
		 * @brief Load internal modules.
		 */
		void load_modules();
	};

}
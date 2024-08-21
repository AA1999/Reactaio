//
// Created by arshia on 8/19/24.
//

#pragma once
#include <dlfcn.h>
#include <filesystem>

namespace reactaio::internal {
	class module_wrapper {
		void* m_handler;
		std::filesystem::path m_path;
	public:
		explicit module_wrapper(const std::filesystem::path& path): m_handler(dlopen(path.c_str(), RTLD_GLOBAL)), m_path(path) {};
		module_wrapper() = delete;

		/**
		 * @brief Get module path.
		 * @return Path to the .so module.
		 */
		[[nodiscard]] constexpr std::filesystem::path path() const {
			return m_path;
		}

		/**
		 * @brief Get the library handler pointer.
		 * @return The .so handler with the dlopen()
		 */
		[[nodiscard]] constexpr void* handler() const {
			return m_handler;
		}

		~module_wrapper();
	};
}
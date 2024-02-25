//
// Created by arshia on 2/25/24.
//

#include <string_view>

export module split_proxy;

import reactaio.internal.strings;

export namespace reactaio::internal {
	/**
	 * @brief split_proxy - A wrapper for a series of iterators for split strings.
	 * @tparam CharT Character type (wstring_view, string_view etc.)
	 */
	template <typename CharT>
	class split_proxy {
		using string_view = std::basic_string_view<CharT>;

		string_view m_string;
		string_view m_delimiter;
	public:
		using iterator = splitting_iterator<CharT>;

		/**
		 * @brief The constructor to create the proxy from a string and a delimiter
		 * @param string Target string to split.
		 * @param delimiter The delimiter to split by.
		 */
		split_proxy(string_view string, string_view delimiter): m_string(string), m_delimiter(delimiter){}


		/**
		 * @brief begin - The beginning of the split strings
		 * @return Iterator to the beginning of the split strings.
		 */
		iterator begin() {
			return iterator(m_string, m_delimiter, 0);
		}

		/**
		 * @brief end - The end of the split strings
		 * @return Iterator to the end of the split strings.
		 */
		iterator end() {
			return iterator(m_string, m_delimiter, string_view::npos);
		}

		/**
		 * @brief front - First split string
		 * @return Reference to the first split string.
		 */
		string_view front() {
			return *begin();
		}

		/**
		 * @brief back - Last split string
		 * @return Reference to the last split string.
		 */
		string_view back() {
			return *end();
		}
	};
}
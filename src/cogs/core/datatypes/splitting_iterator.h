//
// Created by arshia on 1/25/24.
//

#pragma once

#include <string_view>

namespace reactaio::internal {
	/**
	 * @brief spitting_iterator - An iterator for a split string
	 * @tparam CharT Character type (wstring_view, string_view etc.)
	 */
	template<typename CharT>
	class splitting_iterator {
		using string_view = std::basic_string_view<CharT>;

		string_view m_string;
		string_view m_delimiter;
		std::size_t m_pos;
		std::size_t m_next;

	public:
		/**
		 * @brief The constructor to construct the iterators from a string view and a delimiter
		 * @param string The string to split.
		 * @param delimiter The delimiter to split by.
		 * @param pos Initial position, default should be 0.
		 */
		splitting_iterator(string_view string, string_view delimiter, std::size_t pos): m_string(string), m_delimiter(delimiter), m_pos(pos), m_next(m_string.find(m_delimiter, m_pos)){}

		/**
		 * @brief operator++ - Moves to the next iterator.
		 */
		void operator++() {
			m_pos = m_string.find(m_delimiter, m_pos) + 1;
			m_next = m_string.find(m_delimiter, m_pos);
		}

		/**
		 * @brief operator* - De-references the current iterator.
		 * @return The underlying value of the iterator.
		 */
		string_view operator* () {
			if(m_next == string_view::npos)
				return string_view{m_string.begin() + m_pos, m_string.end()};
			return string_view{m_string.begin() + m_pos, m_string.begin() + m_next};
		}

		/**
		 * @brief operator == - Equality comparison operator.
		 * @param other Another iterator to compare this to.
		 * @return true if the string and delimiters and the position are the same.
		 * @return false if the string and the delimiters and the position aren't the same.
		 */
		bool operator== (splitting_iterator const& other) const {
			return this->m_string == other.m_string && this->m_delimiter == other.m_delimiter && this->m_pos == other.m_pos;
		}
	};
}


//
// Created by arshia on 2/24/23.
//

#pragma once

#include <initializer_list>
#include <optional>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <format>

namespace reactaio::internal {
	/**
	 * @brief fifo_map - A map that remembers insertion order.
	 * @tparam K Type of the keys.
	 * @tparam V Type of the values.
	 */
	template <typename K, typename V>
	class fifo_map {
		std::vector<K> m_keys;
		std::vector<V> m_values;

		/**
		 * @brief find - Finds the element associated with the given key.
		 * @param key The key of the intended element.
		 * @return The element that matches the key. If it doesn't exist, returns std::nullopt instead.
		 */
		constexpr std::optional<std::ptrdiff_t> find(const K& key) const {
			const auto iterator = std::ranges::find(m_keys, key);
			return iterator != m_keys.end() ? std::optional(std::distance(m_keys.begin(), iterator)) : std::nullopt;
		}


	public:
		fifo_map(const std::initializer_list<std::pair<K, V>> initializer_list) {
			for(auto const& [key, value]: initializer_list)
				insert(key, value);
		}

		fifo_map() = default;

		/**
		 * @brief insert - Inserts a new element into the fifo map.
		 * @param key The key representing the element.
		 * @param value The value of that element.
		 */
		void insert(const K& key, const V & value) {
			if(auto key_index = find(key)) {
				auto index = key_index.value();
				m_values[index] = value;
			}
			else {
				m_keys.push_back(key);
				m_values.push_back(value);
			}
 		}

		/**
		 * @brief pop - Removes the element with the specified key from the fifo map.
		 * @param key The key of the element that is supposed to be removed.
		 * @return The value of the element that was removed.
		 */
		constexpr std::optional<K> pop(const K& key) {
			if(auto key_index = find(key)) {
				auto index = key_index.value();
				m_keys.erase(index);
				return m_values.erase(index);
			}
			return std::nullopt;
		}

		/**
		 * @brief size - Size of the FIFO map.
		 * @return An integer that's the length of this map.
		 */
		[[nodiscard]] constexpr std::size_t size() const {
			return m_keys.size();
		}

		/**
		 * @brief operator[] - Returns the element that's associated with the given key.
		 * @param key - Key for the lookup.
		 * @return The element associated with the key.
		 */
		constexpr V & operator[](const K& key) const {
			if(auto key_index = find(key)) {
				auto index = key_index.value();
				return m_values[index];
			}
		}

		/**
		 * @brief Finds the key pointing to the given value.
		 * @param value The value to look up.
		 * @return The key that points to the value.
		 */
		 const K& find_key(const V& value) const {
			auto iterator = std::ranges::find(m_values, value);
			if(iterator != m_values.end()) {
				auto const index = std::distance(begin(m_values), iterator);
				return m_keys.at(index);
			}
			throw std::out_of_range{std::vformat("Lookup of key to value {} failed, value doesn't exist in map.", std::make_format_args(value))};
		}

		/**
		 * @brief at - Returns the element that's associated with the given key. Throws an error if not found.
		 * @param key - Key for the lookup.
		 * @return The element associated with the key.
		 * @throw std::out_of_range If the key doesn't exist.
		 */
		constexpr V& at(const K& key) const {
			if(auto key_index = find(key)) {
				auto index = key_index.value();
				return m_values.at(index);
			}
			throw std::out_of_range{std::format("Lookup of key {} failed, key doesn't exist in map.", key)};
		}

		/**
		 * @brief keys - Returns all the keys of this map.
		 * @return A std::vector consisting of all the keys in this map.
		 */
		std::vector<K> keys() const {
			return m_keys;
		}

		/**
		 * @brief values - Returns all the values of the map.
		 * @return A std::vector consisting of all the keys in this map.
		 */
		std::vector<V> values() const {
			return m_values;
		}

	};
}
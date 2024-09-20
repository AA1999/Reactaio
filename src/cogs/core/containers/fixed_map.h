//
// Created by arshia on 12/30/23.
//

#pragma once

#include <unordered_map>
#include <array>
#include <initializer_list>
#include <functional>
#include <stdexcept>
#include <format>
#include <cassert>

namespace reactaio::internal {
	/**
	 * @brief An unordered map with a static size.
	 * @tparam Key Type of the map keys.
	 * @tparam Value Type of the map values.
	 * @tparam size size of the map.
	 * @tparam Hash Hash type for each element of the map.
	 * @tparam KeyEqual The algorithm to compare key equals.
	 * @tparam Allocator Type of the allocator for the map.
	 */
	template <typename Key, typename Value, std::size_t size, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
	class fixed_map {

		struct value_type {
			Key key;
			Value value;
		};

		/**
		 *
		 * @param list Initializer list to convert to a std::array
		 * @return An array of
		 */
		static constexpr std::array<value_type, size> to_array(const std::initializer_list<value_type>& list) {
			if(list.size() > size)
				throw std::out_of_range{"The given initializer list is bigger than the allocated size for the map."};
			return ([list]<std::size_t... i>(std::index_sequence<i...>) {
				auto iterator = list.begin();
				return std::array<value_type, size> {
					(void(i), *iterator++)...
				};
			})(std::make_index_sequence<size>());
		}

		mutable std::array<value_type, size> m_array;

		/**
		 * @brief Get the internal map.
		 * @return The underlying unordered map.
		 */
		[[nodiscard]] std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& get_map() const noexcept {
			static std::unordered_map<Key, Value, Hash, KeyEqual, Allocator> map {m_array.begin(), m_array.end()};
			return map;
		}

		using iterator = typename std::array<value_type, size>::iterator;
		using const_iterator = typename std::array<value_type, size>::const_iterator;
		using key_type = Key;

		/**
		 * @brief Internal key lookup method.
		 * @param key Key to lookup.
		 * @param check Check for out of bounds or not.
		 * @return The value corresponding to the given key. Throws an exception if the check parameter is set to true.
		 */
		[[nodiscard]] constexpr Value lookup(const Key& key, const bool check = true) const {
			if (std::is_constant_evaluated()) {
				auto find = std::ranges::find(m_array, key, [key](value_type const &element) {
					return element.key == key;
				});
				if (find != m_array.end()) {
					return find;
				}
				if(check)
					throw std::out_of_range{std::format("Key {} doesn't exist in the map.", key)};
				assert(false);
			}
			if (check)
				return get_map().at(key);
			return get_map()[key];
		}

	public:
		fixed_map() noexcept = default;
		~fixed_map() = default;

		/**
		 * @brief Creates a fixed map from an initializer list.
		 * @param list The initializer to fill the map with.
		 */
		constexpr fixed_map(const std::initializer_list<value_type> list): m_array(to_array(list)) {}

		/**
		 * @brief operator[] - looks up a key in the map.
		 * @param key The key to lookup
		 * @return The value representing the key,
		 */
		[[nodiscard]] constexpr const Value& operator [] (const Key& key) const {
			return lookup(key, false);
		}

		/**
		 * @brief Looks up a key in the map - non-const version
		 * @param key The key to lookup.
		 * @return The value associated with the key
		 */
		[[nodiscard]] constexpr Value& operator[](const Key& key) {
			return lookup(key, false);
		}

		/**
		 * @brief at - Looks up a key in the map.
		 * @param key The key to lookup.
		 * @return The value associated with the key.
		 * @throw std::out_of_range if the key is not found.
		 */
		constexpr const Value& at(const Key& key) const {
			return lookup(key);
		}

		/**
		 * @brief begin - Returns an iterator to the beginning of the map.
		 * @return Iterator pointing to the first element.
		 */
		[[nodiscard]] constexpr const_iterator begin() const noexcept {
			return m_array.begin();
		}

		/**
		 * @brief begin - Returns a const iterator to the beginning of the map.
		 * @return Const iterator pointing to the first element.
		 */
		[[nodiscard]] constexpr iterator begin() noexcept {
			return m_array.begin();
		}

		/**
		 * @brief begin - Returns an iterator to the beginning of the map.
		 * @return Iterator pointing to the first element.
		 */
		[[nodiscard]] constexpr const_iterator end() const noexcept {
			return m_array.end();
		}

		/**
		 * @brief begin - Returns an iterator to the end of the map.
		 * @return Iterator pointing to the last element.
		 */
		[[nodiscard]] constexpr iterator end() noexcept {
			return m_array.end();
		}

		/**
		 * @brief begin - Returns a const version of the first element of the map.
		 * @return The first element of the map as const.
		 */
		constexpr const Value& front() const noexcept {
			return *begin();
		}

		/**
		 * @brief begin - Returns the first element of the map.
		 * @return The first element of the map.
		 */
		constexpr Value& front() noexcept {
			return *begin();
		}

		/**
		 * @brief begin - Returns a const version of the last element of the map.
		 * @return The first element of the map as const.
		 */
		constexpr const Value& back() const noexcept {
			return *end();
		}

		/**
		 * @brief begin - Returns the last element of the map.
		 * @return The last element of the map.
		 */
		constexpr Value& back()  noexcept {
			return *end();
		}

		/**
		 * @brief contains - Checks if the map contains a key.
		 * @param key The key to check its existence.
		 * @return true if the map has the key.
		 * @return false if the map doesn't have the key.
		 */
		[[nodiscard]] constexpr bool contains(const Key& key) {
			if(std::is_constant_evaluated()) {
				for(auto& [array_key, value]: m_array) {
					if(array_key == key)
						return true;
				}
				return false;
			}
			return get_map().contains(key);
		}

		/**
		 * @brief data - Returns the map data.
		 * @return An array of all the key/value pairs.
		 */
		[[nodiscard]] constexpr std::array<value_type, size> data() const noexcept {
			return m_array;
		}

	};
} // namespace reactaio::internal
//
// Created by arshia on 2/24/23.
//
// FIFO Map: A map that remembers insertion order.

#pragma once

#include <dpp/dpp.h>
#include <initializer_list>
#include <vector>
#include <algorithm>

namespace reactaio::internal {
	template <typename K, typename V>
	class fifo_map {
		std::vector<K> _keys;
		std::vector<V> _values;

		auto index(const K & key) const {
			const auto iterator = std::find(begin(_keys), end(_keys), key);
			return iterator != end(_keys) ? std::optional(std::distance(begin(_keys), iterator)) : std::nullopt;
		}


	public:
		fifo_map(const std::initializer_list<std::pair<K, V>> initializer_list) {
			for(auto const& [key, value]: initializer_list)
				insert(key, value);
		}

		fifo_map() = default;

		void insert(const K & key, const V & value) {
			if(auto key_index = index(key)) {
				auto index = key_index.value();
				_values[index] = value;
			}
			else {
				_keys.push_back(key);
				_values.push_back(value);
			}
 		}

		void pop(const K & key) {
			if(auto key_index = index(key)) {
				auto index = key_index.value();
				_keys.erase(index);
				_values.erase(index);
			}
		}

		std::size_t size() const {
			return _keys.size();
		}

		const V & operator[](const K & key) const {
			if(auto key_index = index(key)) {
				auto index = key_index.value();
				return _values[index];
			}
			insert(key, V());
			return _values.end();
		}

		const std::vector<K> keys() const {
			return _keys;
		}

		const std::vector<V> values() const {
			return _values;
		}

	};
}
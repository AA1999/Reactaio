//
// Created by arshia on 3/30/24.
//

#pragma once

#include <algorithm>
#include <vector>

namespace reactaio::internal {
	/**
	 * @brief A vector with all unique elements.
	 * @note The reason a std::set/std::unordered_set isn't used is because of its heavy memory usage.
	 * @tparam T Type of the container data.
	 */
	template<typename T>
	class unique_vector {
		std::vector<T> m_container;

	public:
		using iterator = typename std::vector<T>::iterator;
		using const_iterator = typename std::vector<T>::const_iterator;
		using reference = typename std::vector<T>::reference;
		using const_reference = typename std::vector<T>::const_reference;
		using pointer = typename std::vector<T>::pointer;
		using const_pointer = typename std::vector<T>::const_pointer;
		using size_type = typename std::vector<T>::size_type;
		using difference_type = std::iter_difference_t;
		using value_type = T;

		/**
		 * @brief Beginning of the vector.
		 * @return An iterator to the first element.
		 */
		[[nodiscard]] constexpr iterator begin() {
			return m_container.begin();
		}


		/**
		 * @brief Beginning of the vector.
		 * @return A const iterator to the first element.
		 */
		[[nodiscard]] constexpr const_iterator begin() const {
			return m_container.begin();
		}


		/**
		 * @brief End of the vector.
		 * @return An iterator to the last element.
		 */
		[[nodiscard]] constexpr iterator end() noexcept {
			return m_container.end();
		}

		/**
		 * @brief End of the vector.
		 * @return A const iterator to the last element.
		 */
		[[nodiscard]] constexpr const_iterator end() const noexcept {
			return m_container.end();
		}

		/**
		 * @brief First element of the vector.
		 * @return A reference to the first element.
		 */
		[[nodiscard]] constexpr reference front() {
			return m_container.front();
		}

		/**
		 * @brief First element of the vector.
		 * @return A const reference to the first element.
		 */
		[[nodiscard]] constexpr const_reference front() const {
			return m_container.front();
		}

		/**
		 * @brief Last element of the vector.
		 * @return A reference to the last element.
		 */
		[[nodiscard]] constexpr reference back() {
			return m_container.back();
		}

		/**
		 * @brief Last element of the vector.
		 * @return A const reference to the last element.
		 */
		[[nodiscard]] constexpr const_reference back() const {
			return m_container.back();
		}


		/**
		 * @brief How many elements does the vector have?
		 * @return The size of the vector.
		 */
		[[nodiscard]] constexpr size_type size() const noexcept {
			return m_container.size();
		}


		/**
		 * @brief How many elements can be added until it can no longer hold any?
		 * @return The maximum size that this container can have according to the library implementation.
		 */
		[[nodiscard]] constexpr size_type max_size() const noexcept {
			return m_container.max_size();
		}

		/**
		 * @brief Allocated capacity.
		 * @return The maximum elements this container can have before needing to reallocate memory.
		 */
		[[nodiscard]] constexpr size_type capacity() const noexcept {
			return m_container.capacity();
		}

		/**
		 * @brief Returns true if the container has no elements.
		 * @return true if the container is empty
		 * @return false if the container is not empty
		 */
		[[nodiscard]] constexpr bool empty() const noexcept {
			return m_container.empty();
		}

		/**
		 * @brief Swap elements with other container.
		 * @param other Other container to swap with.
		 */
		constexpr void swap(unique_vector& other) noexcept {
			m_container.swap(other.m_container);
		}

		/**
		 * @brief Appends the given element value to the container so that it's sorted.
		 * @param value The value to insert inside the vector.
		 */
		constexpr void insert(value_type const &value) {
			auto iter = std::ranges::lower_bound(m_container, value);
			if(*iter != value)
				m_container.insert(iter, value);
		}

		/**
		 * @brief Moves the given element to the end of the container.
		 * @param value The value to be moved inside the vector.
		 */
		constexpr void insert(value_type &&value) {
			auto iter = std::ranges::lower_bound(m_container, value);
			if(*iter != value)
				m_container.insert(iter, std::move(value));
		}

		/**
		 * @brief Appends a new element to the the container which is constructed with the given arguments so that it's sorted.
		 * @tparam Args Template parameters
		 * @param args Arguments to call the constructor of the value_type from.
		 */
		template <typename... Args>
		constexpr void insert(Args&&... args) {
			value_type value(std::forward<Args>(args)...);
			auto iter = std::ranges::lower_bound(m_container, value);
			if(*iter != value)
				m_container.insert(iter, std::move(value));
		}

		/**
		 * @brief Removes the last element of the container.
		 * @return A reference to the last element that was removed.
		 */
		constexpr reference pop_back() {
			auto pop = front();
			m_container.pop_back();
			return pop;
		}

		/**
		 * @brief Removes all elements of the container.
		 */
		constexpr void clear() noexcept {
			m_container.clear();
		}

	};
}
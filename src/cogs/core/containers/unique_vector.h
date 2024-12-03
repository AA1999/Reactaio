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
	 * @note This container is always sorted.
	 */
	template<typename T>
	class unique_vector {
		std::vector<T> m_container;
	public:
		using iterator = typename std::vector<T>::iterator;
		using const_iterator = typename std::vector<T>::const_iterator;
		using reference = typename std::vector<T>::reference;
		using const_reference = typename std::vector<T>::const_reference;
		using size_type = typename std::vector<T>::size_type;
		using difference_type = typename std::vector<T>::difference_type;
		using value_type = T;

		~unique_vector() = default;
		unique_vector(): m_container(){}

		/**
		 * @brief Copy constructor.
		 * @param other Other unique vector to copy.
		 */
		unique_vector(const unique_vector &other): m_container(other.m_container) {}

		/**
		 * @brief Move constructor.
		 * @param other Other unique vector to move.
		 */
		unique_vector(unique_vector&& other) noexcept: m_container(std::move(other.m_container)) {}

		/**
		 * @brief Copy constructor.
		 * @tparam R Type of the range.
		 * @param range The range to copy from.
		 */
		template <typename R>
		requires std::ranges::range<R>
		unique_vector(const R& range) {
			insert_range(range);
		}

		/**
		 * @brief Move constructor.
		 * @tparam R Type of the range.
		 * @param range The range to move.
		 */
		template <typename R>
		requires std::ranges::range<R>
		unique_vector(R&& range) {
			insert_range(range);
		}

		/**
		 * @brief Copy assignment operator.
		 * @param other Other container to assign this to.
		 * @return A reference to the updated instance.
		 */
		unique_vector & operator =(const unique_vector &other) {
			if (this == &other)
				return *this;
			m_container = other.m_container;
			return *this;
		}

		/**
		 * @brief Assignment operator.
		 * @param other Other container to assign to this.
		 * @return A reference to the updated instance.
		 */
		unique_vector & operator =(unique_vector &&other) noexcept {
			if (this == &other)
				return *this;
			m_container = std::move(other.m_container);
			return *this;
		}

		/**
		 * @brief Assignment operator.
		 * @tparam R Type of the given range.
		 * @param range Range to assign to this container.
		 * @return A reference to this instance.
		 */
		template <typename R>
		requires std::ranges::range<R>
		constexpr unique_vector& operator =(R&& range) {
			clear();
			insert_range(range);
			return *this;
		}

		/**
		 * @brief Equality comparison operator.
		 * @param other Object to compare this to.
		 * @return true if the underlying containers are equal.
		 * @return false if the underlying containers are not equal.
		 */
		[[nodiscard]] constexpr bool operator ==(const unique_vector& other) const {
			return m_container == other.m_container;
		}

		/**
		 * @brief Inequality comparison operator.
		 * @param other Object to compare this to.
		 * @return true if the underlying containers are not equal.
		 * @return false if the underlying containers are equal.
		 */
		[[nodiscard]] constexpr bool operator !=(const unique_vector& other) const {
			return !(this == other);
		}

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
		 * @brief Beginning of the vector.
		 * @return A const iterator to the first element.
		 */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept {
			return m_container.cbegin();
		}

		/**
		 * @brief End of the vector.
		 * @return An iterator to the last element.
		 */
		[[nodiscard]] constexpr iterator end() {
			return m_container.end();
		}

		/**
		 * @brief End of the vector.
		 * @return A const iterator to the last element.
		 */
		[[nodiscard]] constexpr const_iterator end() const {
			return m_container.end();
		}

		/**
		 * @brief End of the vector.
		 * @return A const iterator to the last element.
		 */
		[[nodiscard]] constexpr const_iterator cend() noexcept {
			return m_container.cend();
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
		 * @note This function is called push_back due to the requirement for compatibility with std::back_inserter.
		 */
		constexpr void insert(value_type const &value) {
			auto iter = std::ranges::lower_bound(m_container, value);
			if(*iter != value)
				m_container.emplace(iter, value);
		}

		/**
		 * @brief Moves the given element to the end of the container.
		 * @param value The value to be moved inside the vector.
		 * @note This function is called push_back due to the requirement for compatibility with std::back_inserter.
		 */
		constexpr void insert(value_type &&value) {
			auto iter = std::ranges::lower_bound(m_container, value);
			if(*iter != value)
				m_container.emplace(iter, std::move(value));
		}

		/**
		 * @brief Appends a new element to the container which is constructed with the given arguments so that it's sorted.
		 * @tparam Args Template parameters
		 * @param args Arguments to call the constructor of the value_type from.
		 */
		template <typename... Args>
		constexpr void insert(Args&&... args) {
			value_type value(std::forward<Args>(args)...);
			auto iter = std::ranges::lower_bound(m_container, value);
			if(*iter != value)
				m_container.emplace(iter, std::move(value));
		}

		/**
		 * @brief Places the given range inside the container except the common elements.
		 * @tparam R Type of the range.
		 * @param range The given range to append.
		 */
		template <typename R>
		requires std::ranges::range<R>
		constexpr void insert_range(R&& range) {
			for(auto const& element: range)
				insert(element);
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

		/**
		 * @brief Reverses the order that the elements of the vector are sorted.
		 */
		constexpr void reverse() noexcept {
			std::ranges::reverse(m_container);
		}

		/**
		 * @brief Finds the position of the given value in the container.
		 * @param value The value to look up.
		 * @return The iterator to the found element.
		 */
		[[nodiscard]] constexpr iterator find(const value_type& value) {
			return std::ranges::find(m_container, value);
		}

		/**
		 * @brief Finds the position of the given value in the container.
		 * @param value The value to look up.
		 * @return A const iterator to the found element.
		 */
		[[nodiscard]] constexpr const_iterator find(const value_type& value) const {
			return std::ranges::find(m_container, value);
		}

		/**
		 * @brief Does the container contain this value?
		 * @param value The value to lookup in the container.
		 * @return true if the container contains the given value.
		 * @return false if the container doesn't contain the given value.
		 */
		[[nodiscard]] constexpr bool contains(const value_type& value) const {
			return find(value) != end();
		}

		/**
		 * @brief Removes element at the given position.
		 * @param position Iterator to the element being removed.
		 * @return Iterator after the removed element.
		*/
		[[nodiscard]] constexpr iterator erase(const_iterator position) {
			return m_container.erase(position);
		}

		/**
		 * 
		 * @param first Start of the range that is going to be removed.
		 * @param last End of the range that's going to be removed.
		 * @return end() if last == end()
		 * @return last if the given range is empty
		 */
		[[nodiscard]] constexpr iterator erase(const_iterator first, const_iterator last) {
			return m_container.erase(first, last);
		}

		/**
		 * @brief Removes the given value from the container.
		 * @param value Value to be removed from the container.
		 */
		constexpr void erase(const value_type& value) {
			std::erase(m_container, value);
		}

		/**
		 * @brief Random access lookup function.
		 * @param index The element index
		 * @return Reference to the element at that index.
		 * @throw std::out_of_range on out of bounds.
		 */
		[[nodiscard]] constexpr reference at(size_type index) {
			return m_container.at(index);
		}

		/**
		 * @brief Random access lookup function.
		 * @param index The element index
		 * @return Const reference to the element at that index.
		 * @throw std::out_of_range on out of bounds.
		 */
		[[nodiscard]] constexpr const_reference at(size_type index) const {
			return m_container.at(index);
		}

		/**
		 * @brief Random access lookup operator. No bounds checking.
		 * @param index The index for the lookup.
		 * @return Reference to the element at that index.
		 */
		[[nodiscard]] reference operator[](size_type index) {
			return m_container[index];
		}

		/**
		 * @brief Random access lookup operator. No bounds checking.
		 * @param index The index for the lookup.
		 * @return Const reference to the element corresponding to the index.
		 */
		[[nodiscard]] constexpr const_reference operator [](size_type index) const {
			return m_container[index];
		}

		/**
		 * @brief Increases the capacity (How many elements it can hold without memory reallocation) of the vector to the given number.
		 * @param new_size The size to reserve for the vector.
		 */
		constexpr void reserve(size_type new_size) {
			m_container.reserve(new_size);
		}

		/**
		 * @brief Resizes the container to the given size. Does nothing if new_size == size().
		 * @param new_size New size of the container.
		 */
		constexpr void resize(size_type new_size) {
			m_container.resize(new_size);
		}
		
		/**
		 * @brief Resizes the container with the given values until the size is increased to the given size. Does nothing if new_size == size().
		 * @param new_size New size of the container.
		 * @param value The value to insert until the size is new_size.
		 */
		constexpr void resize(size_type new_size, const T& value) {
			m_container.resize(new_size, value);
		}
	};
}
//
// Created by arshia on 4/19/24.
//

#pragma once

#include "datatypes/unique_vector.h"

namespace reactaio {
	/**
	 * @brief Copies all the elements from the first unique vector to the second one.
	 * @tparam T Type of the unique vectors.
	 * @param input Unique vector to copy from.
	 * @param output Unique vector to copy to.
	 */
	template <typename T>
	constexpr void copy(internal::unique_vector<T> const& input, internal::unique_vector<T>& output) {
		output.insert_range(input);
	}

	/**
	 * @brief Copies any of the elements from the first unique vector to the second one that the given condition applies to.
	 * @tparam T Type of the unique vectors.
	 * @tparam UnaryPred A unary predicate is any custom struct/function/lambda that can be used to set a condition for the copy_if.
	 * @param input Unique vector to copy from.
	 * @param output Unique vector to copy to.
	 * @param pred The condition for the copy.
	 */
	template <typename T, typename UnaryPred>
	constexpr void copy_if(internal::unique_vector<T> const& input, internal::unique_vector<T>& output, UnaryPred pred) {
		for(auto const& it = input.begin(); it <= input.end(); ++it)
			if(pred(*it))
				output.insert(*it);
	}

	/**
	 * @brief Moves all the elements from the first unique vector to the second one.
	 * @tparam T Type of the unique vectors.
	 * @param input Unique vector to move from.
	 * @param output Unique vector to move to.
	 */
	template <typename T>
	constexpr void move(internal::unique_vector<T> const& input, internal::unique_vector<T>& output) {
		for(auto const& it  = input.begin(); it <= input.end(); ++it)
			output.insert(std::move(*it));
	}

	/**
	 * @brief Copies all elements from the first range that aren't in the second range inside the given output.
	 * @tparam T Types of the input and output ranges.
	 * @param range1 Range to find the different elements from.
	 * @param range2 Range to avoid copying elements from.
	 * @param output Range to copy the output in.
	 */
	template <typename T>
	constexpr void set_difference(internal::unique_vector<T> const& range1, internal::unique_vector<T> const& range2, internal::unique_vector<T>& output) {
		for(auto const& it = range1.begin(); it <= range1.end(); ++it)
			if(std::ranges::find(range2, *it) == std::ranges::end(range2))
				output.insert(*it);
	}

	/**
	 * @brief Copies all elements from the first range that aren't in the second range and all the elements from the second range that aren't in the first range inside the given output.
	 * @tparam T Types of the input and output ranges.
	 * @param range1 Range to find the different elements from and avoid similar elements.
	 * @param range2 Range to find the different elements from and avoid similar elements..
	 * @param output Range to copy the output in.
	 */
	template <typename T>
	constexpr void set_symmertric_difference(internal::unique_vector<T> const& range1, internal::unique_vector<T> const& range2, internal::unique_vector<T>& output) {
		set_difference(range1, range2, output);
		set_difference(range2, range1, output);
	}

	/**
	 * @brief Transforms all the elements from the first range into the second one.
	 * @tparam T Type of the input range.
	 * @tparam O Type of the output range.
	 * @tparam Proj Custom converter type.
	 * @param input Input range to convert.
	 * @param output Output for the conversion.
	 * @param proj Conversion method, a static cast by default.
	 */
	template <typename T, typename O, typename Proj>
	constexpr void transform(internal::unique_vector<T> const& input, internal::unique_vector<O>& output, Proj proj = [](T const& element){return static_cast<O>(element);}) {
		for(auto const& it = input.begin(); it <= input.end(); ++it)
			output.insert(proj(*it));
	}

	/**
	 * @brief Copies the common elements of the two given ranges into the
	 * @tparam T Type of the input and output range.
	 * @param range1 First range to find common elements in.
	 * @param range2 Second range to find common elements in.
	 * @param output The range to copy the common elements to.
	 */
	template <typename T>
	constexpr void set_intersection(internal::unique_vector<T> const& range1, internal::unique_vector<T> const& range2, internal::unique_vector<T>& output) {
		for(auto const& it = range1.begin(); it <= range1.end(); ++it)
			if(std::ranges::find(range2, *it) != std::ranges::end(range2))
				output.insert(*it);
	}
}
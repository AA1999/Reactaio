//
// Created by arshia on 4/19/24.
//

#pragma once

#include "containers/unique_vector.h"

namespace reactaio {
	/**
	 * @brief Copies all the elements from the first unique vector to the second one.
	 * @tparam T Type of the unique vectors.
	 * @param input Unique vector to copy from.
	 * @param output Unique vector to copy to.
	 */
	template <typename T>
	constexpr void copy(internal::unique_vector<T> const& input, internal::unique_vector<T>& output) {
		output = input;
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
		for(auto const& element: input)
			if(pred(element))
				output.insert(element);
	}

	/**
	 * @brief Moves all the elements from the first unique vector to the second one.
	 * @tparam T Type of the unique vectors.
	 * @param input Unique vector to move from.
	 * @param output Unique vector to move to.
	 */
	template <typename T>
	constexpr void move(internal::unique_vector<T> const& input, internal::unique_vector<T>& output) {
		output = std::move(input);
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
		for(auto const& element : range1)
			if(!range2.contains(element))
				output.insert(element);
	}

	/**
	 * @brief Copies all elements from the first range that aren't in the second range and all the elements from the second range that aren't in the first range inside the given output.
	 * @tparam T Types of the input and output ranges.
	 * @param range1 Range to find the different elements from and avoid similar elements.
	 * @param range2 Range to find the different elements from and avoid similar elements.
	 * @param output Range to copy the output in.
	 */
	template <typename T>
	constexpr void set_symmetric_difference(internal::unique_vector<T> const& range1, internal::unique_vector<T> const& range2, internal::unique_vector<T>& output) {
		constexpr auto intersection = set_intersection(range1, range2);
		set_difference(range1, intersection, output);
		set_difference(range2, intersection, output);
	}

	/**
	 * @brief Transforms all the elements from the first range into the second one.
	 * @tparam T Type of the output range.
	 * @tparam R Input range
	 * @tparam Proj Custom converter type.
	 * @param input Input range to convert.
	 * @param output Output for the conversion.
	 * @param proj Conversion method, a static cast by default.
	 */
	template <typename R, typename T, typename Proj>
	constexpr void transform(R&& input, internal::unique_vector<T>& output, Proj proj = [](T const& element) -> T {return static_cast<T>(element);}) {
		for(auto const& element: input)
			output.insert(proj(element));
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
		for(auto const& element : range1)
			if(range2.contains(element))
				output.insert(element);
	}
}
//
// Created by arshia on 2/16/23.
//

#pragma once

#include "../strings.h"

#include <array>
#include <chrono>
#include <string>
#include <vector>
#include <ranges>

using namespace std::literals;
namespace chr = std::chrono;

ushort const TIME_PARTS_COUNT = 7;

enum time: std::uint8_t {
	years = 0,
	months = 1,
	weeks = 2,
	days = 3,
	hours = 4,
	minutes = 5,
	seconds = 6
};

// clang-format off
const static std::vector<std::pair<chr::seconds, std::vector<std::string>>> units{
    {chr::years  {1}, {"y",  "yr",  "yrs",  "year",   "years"}},
    {chr::months {1}, {      "mo",  "mos",  "month",  "months"}},
    {chr::weeks  {1}, {"w",  "wk",  "wks",  "week",   "weeks"}},
    {chr::days   {1}, {"d",                 "day",    "days"}},
    {chr::hours  {1}, {"h",  "hr",  "hrs",  "hour",   "hours"}},
    {chr::minutes{1}, {"m", "min", "mins", "minute", "minutes"}},
    {chr::seconds{1}, {"s", "sec", "secs", "second", "seconds"}},
};
// clang-format on

struct duration {
	std::array<std::uint64_t, TIME_PARTS_COUNT> values{0};

	/**
	 * @brief years - Returns the years of a duration.
	 * @return The years passed in this duration.
	 */
	std::uint64_t& years() {
		return values.at(time::years);
	}

	/**
	 * @brief months - Returns the months of a duration.
	 * @return The months passed in this duration.
	 */
	std::uint64_t& months() {
		return values.at(time::months);
	}

	/**
	 * @brief weeks - Returns the weeks of a duration.
	 * @return The weeks passed in this duration.
	 */
	std::uint64_t& weeks() {
		return values.at(time::weeks);
	}

	/**
	 * @brief days - Returns the days of a duration.
	 * @return The days passed in this duration.
	 */
	std::uint64_t& days() {
		return values.at(time::days);
	}

	/**
	 * @brief hours - Returns the hours of a duration.
	 * @return The hours passed in this duration.
	 */
	std::uint64_t& hours() {
		return values.at(time::hours);
	}

	/**
	 * @brief minutes - Returns the minutes of a duration.
	 * @return The minutes passed in this duration.
	 */
	std::uint64_t& minutes() {
		return values.at(time::minutes);
	}

	/**
	 * @brief seconds - Returns the seconds of a duration.
	 * @return The seconds passed in this duration.
	 */
	std::uint64_t& seconds() {
		return values.at(time::seconds);
	}

	/**
	 * @brief years - Returns the years of a duration.
	 * @return The years passed in this duration.
	 */
	[[nodiscard]] std::uint64_t years() const {
		return values.at(time::years);
	}

	/**
	 * @brief months - Returns the months of a duration.
	 * @return The months passed in this duration.
	 */
	[[nodiscard]] std::uint64_t months() const {
		return values.at(time::months);
	}

	/**
	 * @brief weeks - Returns the weeks of a duration.
	 * @return The weeks passed in this duration.
	 */
	[[nodiscard]] std::uint64_t weeks() const {
		return values.at(time::weeks);
	}

	/**
	 * @brief days - Returns the days of a duration.
	 * @return The days passed in this duration.
	 */
	[[nodiscard]] std::uint64_t days() const {
		return values.at(time::days);
	}

	/**
	 * @brief hours - Returns the hours of a duration.
	 * @return The hours passed in this duration.
	 */
	[[nodiscard]] std::uint64_t hours() const {
		return values.at(time::hours);
	}

	/**
	 * @brief minutes - Returns the minutes of a duration.
	 * @return The years minutes in this duration.
	 */
	[[nodiscard]] std::uint64_t minutes() const {
		return values.at(time::minutes);
	}

	/**
	 * @brief seconds - Returns the seconds of a duration.
	 * @return The seconds passed in this duration.
	 */
	[[nodiscard]] std::uint64_t seconds() const {
		return values.at(time::seconds);
	}

	duration() = default;

	/**
	 * @brief Constructor used to read the database data
	 * @param duration_string The string duration read from the database query.
	 */
	explicit duration(std::string_view duration_string) {
		auto tokens = split(duration_string, std::string_view{" "});
		for(auto const& token: tokens) {
			auto number = std::stoull(remove_alpha(token));
			auto ending = remove_numeric(token);
			auto find = std::ranges::find_if(units, [ending](std::pair<chr::seconds, std::vector<std::string>> const &element) {
				return element.second.front() == ending || element.second.back() == ending;
			});
			if (find != units.end()) {// Should always be true but a failsafe
				auto index = std::ranges::distance(units.begin(), find);
				values.at(index) = number;
			}
		}
	}


	/**
	 * @brief to_seconds - Converts this object to a std::chrono::seconds object
	 * @return The std::chrono::seconds variant equivalent of this object
	 */
	[[nodiscard]] chr::seconds to_seconds() const {
		return years() * chr::years{1} + months() * chr::months{1} + weeks() * chr::weeks{1}
		+ days() * chr::days{1} + hours() * chr::hours{1} + minutes() * chr::minutes{1}
		+ seconds() * chr::seconds{1};
	}

	/**
	 * @brief to_string - Converts this object to a string format.
	 * @param shortened Should days be converted to d etc in the final string
	 * @return The string format of this object. Like 5d 5m or 5 days 5minutes if not a shortened version.
	 */
	[[nodiscard]] std::string to_string(bool shortened = false) const {
		std::string res;

		for (int i = time::years; i <= time::seconds; ++i) {
			if (values.at(i) == 0)
				continue;
			if (!res.empty())
				res += " ";
			res += std::to_string(values.at(i)) + " "
				+ (shortened ? units[i].second.front() : units[i].second.back());
		}

		return res;
	}

	/**
	 * @brief since_epoch - Converts this object to a std::time_t one.
	 * @return The std::time_t equivalent of this object.
	 */
	[[nodiscard]] std::time_t since_epoch() const {
		return to_seconds().count();
	}

	/**
	 * @brief operator= - The assignment operator
	 * @param dur The new values for the duration.
	 * @return Should be *this
	 */
	duration& operator=(const duration& dur) = default;
};
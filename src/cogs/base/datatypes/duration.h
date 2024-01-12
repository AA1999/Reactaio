//
// Created by arshia on 2/16/23.
//

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <vector>

using namespace std::literals;
namespace chr = std::chrono;

// enum time {
//     seconds = 0,
//     minutes = 1,
//     hours = 2,
//     days = 3,
//     weeks = 4,
//     months = 5,
//     years = 6
// };

enum time {
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
	std::array<int, 7> values{0};

	int& years() {
		return values.at(time::years);
	}
	int& months() {
		return values.at(time::months);
	}
	int& weeks() {
		return values.at(time::weeks);
	}
	int& days() {
		return values.at(time::days);
	}
	int& hours() {
		return values.at(time::hours);
	}
	int& minutes() {
		return values.at(time::minutes);
	}
	int& seconds() {
		return values.at(time::seconds);
	}

	[[nodiscard]] int years() const {
		return values.at(time::years);
	}
	[[nodiscard]] int months() const {
		return values.at(time::months);
	}
	[[nodiscard]] int weeks() const {
		return values.at(time::weeks);
	}
	[[nodiscard]] int days() const {
		return values.at(time::days);
	}
	[[nodiscard]] int hours() const {
		return values.at(time::hours);
	}
	[[nodiscard]] int minutes() const {
		return values.at(time::minutes);
	}
	[[nodiscard]] int seconds() const {
		return values.at(time::seconds);
	}

	duration() = default;

	[[nodiscard]] chr::seconds to_seconds() const {
		return years() * chr::years{1} + months() * chr::months{1} + weeks() * chr::weeks{1}
		+ days() * chr::days{1} + hours() * chr::hours{1} + minutes() * chr::minutes{1}
		+ seconds() * chr::seconds{1};
	}

	[[nodiscard]] std::string to_string(bool shortened = false) const {
		std::string res;

		for (int i = time::years; i <= time::seconds; ++i) {
			if (values.at(i) == 0) continue;
			if (!res.empty()) res += " ";
			res += std::to_string(values.at(i)) + " "
				+ (shortened ? units[i].second.front() : units[i].second.back());
		}

		return res;
	}

	std::time_t since_epoch() const {
		return to_seconds().count();
	}

	duration& operator=(const duration& dur) = default;
};
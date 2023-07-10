//
// Created by arshia on 2/25/23.
//

#pragma once

#include <pqxx/pqxx>
#include <string>
#include <utility>

struct parser_item {
	pqxx::array_parser::juncture juncture;
	std::string value;
	parser_item() = default;
	explicit parser_item(const std::pair<pqxx::array_parser::juncture, std::string>& array_item): juncture(array_item.first),
	value(array_item.second) {}
};
//
// Created by arshia on 2/25/24.
//

#include <pqxx/pqxx>
#include <string>
#include <utility>

export module parser_item;

export namespace reactaio::internal {
	struct parser_item {
		pqxx::array_parser::juncture juncture;
		std::string value;

		parser_item() = delete;


		/**
	 * @brief The constructor used to split the value and array parser juncture
	 * @param array_item The type of each psql query array element.
	 */
		explicit parser_item(const std::pair<pqxx::array_parser::juncture, std::string>& array_item): juncture(array_item.first),
																									   value(array_item.second) {}
	};
} // namespace reactaio::internal
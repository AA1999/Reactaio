#pragma once

#include <dpp/dpp.h>
#include <string_view>

namespace reactaio::internal {
	class ban {
		dpp::snowflake _user_id;
		dpp::guild* _guild;
		std::string_view _reason;
	public:
		ban() = delete;
		~ban() = default;
		ban(dpp::guild* guild, const dpp::ban& ban): _guild(guild), _reason(ban.reason), _user_id(ban.user_id){}

		/**
		 * @brief guild - The guild corresponding to the ban
		 * @return A pointer to the guild object.
		 */
		[[nodiscard]] dpp::guild* guild() const;

		/**
		 * @brief user_id - The id of the banned user.
		 * @return A dpp::snowflake as the user id.
		 */
		[[nodiscard]] dpp::snowflake user_id() const;

		/**
		 * @brief reason - The reason for the ban.
		 * @return A std::string_view as the ban reason.
		 */
		[[nodiscard]] std::string_view reason() const;

		/**
		 * @brief operator== - Checks if the ban is the same as the other ban.
		 * @param other The second ban compared to this object.
		 * @return true if the guild and userid are equal, false otherwise.
		 */
		bool operator == (const ban & other) const;
	};
} // namespace reactaio::internal
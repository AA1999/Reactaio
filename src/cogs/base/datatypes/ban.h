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

		[[nodiscard]] dpp::guild* guild() const;
		[[nodiscard]] dpp::snowflake user_id() const;
		[[nodiscard]] std::string_view reason() const;

		bool operator == (const ban & other) const;
	};
} // namespace reactaio::internal
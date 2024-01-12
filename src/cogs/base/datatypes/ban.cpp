#include "ban.h"

namespace reactaio::internal {
	dpp::guild*ban::guild() const {
		return _guild;
	}
	
	dpp::snowflake ban::user_id() const {
		return _user_id;
	}

	std::string_view ban::reason() const {
		return _reason;
	}

	bool ban::operator == (const ban &other) const {
		return user_id() == other.user_id() && guild() == other.guild();
	}
} // namespace reactaio::internal
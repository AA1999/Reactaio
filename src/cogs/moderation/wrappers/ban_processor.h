//
// Created by arshia on 4/18/23.
//

#pragma once

#include "../../base/datatypes/ban.h"

#include <vector>
#include <dpp/dpp.h>


class ban_processor {
	dpp::guild* guild;
	std::vector<reactaio::internal::ban> bans;

public:
	explicit ban_processor(dpp::guild* guild): guild(guild), bans(){}

};

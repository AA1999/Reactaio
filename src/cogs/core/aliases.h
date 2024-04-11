//
// Created by arshia on 2/15/23.
//

#pragma once

#include <cstdint>
#include <variant>
#include <dpp/dpp.h>

#include "datatypes/unique_vector.h"

using ullong	  = unsigned long long int;
using ushort	  = unsigned short;
using warn_t	  = ullong;
using case_t	  = ullong;
using member_t	  = ullong;
using duration_t  = ullong;
using count_t	  = ullong;
using snowflake_t = std::uint64_t;
using color_t     = std::uint32_t;

template <typename T>
using shared_set = reactaio::internal::unique_vector<std::shared_ptr<T>>;

using member_user_variant_set = shared_set<std::variant<dpp::guild_member, dpp::user>>;
//
// Created by arshia on 2/15/23.
//

#pragma once

#include <cstdint>
#include <variant>
#include <dpp/dpp.h>

#include "containers/unique_vector.h"

#include <pqxx/pqxx>

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
using shared_vector = reactaio::internal::unique_vector<std::shared_ptr<T>>;

using member_ptr = std::shared_ptr<dpp::guild_member>;
using user_ptr = std::shared_ptr<dpp::user>;
using channel_ptr = std::shared_ptr<dpp::channel>;
using guild_ptr = std::shared_ptr<dpp::guild>;
using connection_ptr = std::unique_ptr<pqxx::connection>;
using interaction_ptr = std::shared_ptr<dpp::slashcommand_t>;
using cluster_ptr = std::shared_ptr<dpp::cluster>;
using role_ptr = std::shared_ptr<dpp::role>;

using member_user_variant = std::variant<member_ptr, user_ptr>;

#define get_name(variable) #variable
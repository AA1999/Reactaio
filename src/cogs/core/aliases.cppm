//
// Created by arshia on 2/25/24.
//

#include <cstdint>

export module reactaio.internal.aliases;

export {
	using ullong	  = unsigned long long int;
	using ushort	  = unsigned short;
	using uint        = unsigned int;
	using warn_t	  = ullong;
	using case_t	  = ullong;
	using member_t	  = ullong;
	using duration_t  = ullong;
	using count_t	  = ullong;
	using snowflake_t = std::uint64_t;
	using color_t     = std::uint32_t;
}
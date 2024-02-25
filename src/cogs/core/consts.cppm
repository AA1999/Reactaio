//
// Created by arshia on 2/25/24.
//

#include <cstdint>

export module reactaio.internal.consts;

import reactaio.internal.aliases;

export {
	inline ushort constexpr bot_max_embed_chars = 4096;
	inline ushort constexpr bot_max_msg_chars = 4000;
	inline uint constexpr max_timeout_seconds = 2419200;
	inline std::uint8_t constexpr bot_max_embeds = 10;
	inline ushort constexpr max_guild_ban_fetch = 1000;
	inline std::uint8_t constexpr max_timeout_days = 28;
}

//
// Created by arshia on 2/15/23.
//

#pragma once

#include <cstdint>

using color_t = uint32_t;

// No ODR violation with `inline`

extern inline color_t const error_color			= 0xFF0000;
extern inline color_t const log_color			= 0x000000;
extern inline color_t const member_remove_color = 0xFF0000;
extern inline color_t const response_color		= 0x00FFCC;
extern inline color_t const warning_color		= 0xFFFF00;
extern inline color_t const success_color		= 0x00FF00;
extern inline color_t const info_color			= 0x0000FF;
extern inline color_t const mute_color			= 0xFFA500;
extern inline color_t const announcements_color = 0x00FF00;
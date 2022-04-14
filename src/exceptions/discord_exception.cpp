//
// Created by arshia on 4/9/22.
//

#include "discord_exception.h"
#include <fmt/format.h>

#include <utility>

discord_exception::discord_exception(discord_exception::code_t code, std::string message) {
    this->code = code;
    this->message = std::move(message);
}

const char *discord_exception::what() noexcept {
    error_print = fmt::format("Code {}: {}", code, message);
    return error_print.c_str();
}

std::string discord_exception::get_message() {
    return message;
}

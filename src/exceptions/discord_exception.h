//
// Created by arshia on 4/9/22.
//

#pragma once

#include <dpp/exception.h>
#include <string>

class discord_exception: dpp::exception {
    using code_t = unsigned long long int;
    std::string error_print;
protected:
    code_t code{};
    std::string message;
public:
    discord_exception() = default;
    discord_exception(code_t code, std::string message);

    std::string get_message();

    virtual const char * what() noexcept;
};
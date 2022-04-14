//
// Created by arshia on 4/11/22.
//

#pragma once

#include "discord_exception.h"

class owner_exception: discord_exception {
public:
    owner_exception();
    explicit owner_exception(const std::string& message);
    const char * what() noexcept override;
};
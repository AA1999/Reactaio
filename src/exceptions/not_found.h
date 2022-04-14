//
// Created by arshia on 4/11/22.
//

#pragma once
#include "discord_exception.h"

class not_found: discord_exception {
public:
    not_found();
    explicit not_found(const std::string& message);
    const char * what() noexcept override;
};


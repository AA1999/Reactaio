//
// Created by arshia on 4/11/22.
//

#pragma once
#include "discord_exception.h"

class missing_permissions: discord_exception {
public:
    missing_permissions();
    explicit missing_permissions(const std::string& message);
    [[nodiscard]] const char* what() noexcept override;
};
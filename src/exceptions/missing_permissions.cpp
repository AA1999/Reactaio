//
// Created by arshia on 4/11/22.
//

#include "missing_permissions.h"

missing_permissions::missing_permissions() {
    code = 403;
    message = "Current user lacks permissions for this action.";
}

missing_permissions::missing_permissions(const std::string& message)  {
    code = 403;
    this->message = message;
}

const char *missing_permissions::what() noexcept {
    return discord_exception::what();
}

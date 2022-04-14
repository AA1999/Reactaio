//
// Created by arshia on 4/11/22.
//

#include "owner_exception.h"

owner_exception::owner_exception() {
    code = 403;
    message = "Cannot perform moderation actions on the server owner.";
}

owner_exception::owner_exception(const std::string& message) {
    code = 403;
    this->message = message;
}

const char *owner_exception::what() noexcept {
    return discord_exception::what();
}

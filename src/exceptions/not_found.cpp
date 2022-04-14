//
// Created by arshia on 4/11/22.
//

#include "not_found.h"

not_found::not_found() {
    code = 404;
    message = "Requested object not found.";
}

not_found::not_found(const std::string &message) {
    code = 404;
    this->message = message;
}

const char *not_found::what() noexcept {
    return discord_exception::what();
}

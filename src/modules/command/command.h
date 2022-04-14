//
// Created by arshia on 4/12/22.
//

#pragma once


#include <dpp/dpp.h>
#include <string>

using std::string;
using dpp::snowflake;
using dpp::guild_member;


struct command {
    const snowflake& channel_id;
    const string& reason;
    string& duration;
    const guild_member& author;
};

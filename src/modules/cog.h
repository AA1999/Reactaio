//
// Created by arshia on 4/11/22.
//

#pragma once

#include <dpp/dpp.h>
#include <pqxx/pqxx>

class cog {
protected:
    dpp::cluster* bot;
    pqxx::work* transcation;
public:
    cog(dpp::cluster& b, pqxx::work& t) : bot(&b), transcation(&t) {}
};
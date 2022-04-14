//
// Created by arshia on 4/9/22.
//

#pragma once

#include "../cog.h"
#include "../command/command.h"
#include <vector>
#include <variant>

using dpp::cluster;
using dpp::snowflake;

using pqxx::work;

using dpp::guild_member;
using dpp::user;
using std::vector;
using std::variant;
using std::string;
using std::get;
using std::holds_alternative;


class moderation: cog {
    using warn_t = unsigned long long int;
    using case_t = unsigned long long int;
    using second_t = unsigned long long int;

    void check_perms(const guild_member& author, const guild_member& target, const snowflake& channel_id);
    void kick_perms_check(const guild_member& author, const snowflake& channel_id);
    void ban_perms_check(const guild_member& author, const snowflake& channel_id);
    void warn_perms_check(const guild_member& author, const snowflake& channel_id);
    void manage_warns_perms_check(const guild_member& author, const snowflake& channel_id);
    void mute_perms_check(const guild_member& author, const snowflake& channel_id);
    void hard_ban_perms_check(const guild_member& author, const snowflake& channel_id);
    static void humanize(string& duration);
    static second_t to_seconds(const string& duration);

public:
    moderation(cluster& bot, work& transaction);

    // Kick

    void kick(const guild_member& member, command& context);
    void kick(const vector<guild_member>& members, command& context);
    // Ban

    void ban(const variant<guild_member, user>& user_or_member, command& context);
    void ban(const vector<variant<guild_member, user>>& users_or_members, const command& context);

    // Soft Ban

    void softban(const variant<guild_member, user>& user_or_member, command& context);
    void softban(const vector<variant<guild_member, user>>& users_or_members, command& context);

    // Hard Ban

    void hardban(const variant<guild_member, user>& user_or_member, command& context);
    void hardban(const vector<variant<guild_member, user>>& users_or_members, command& context);


    // Warn

    void warn(const guild_member& member, command& context);
    void warn(const vector<guild_member>& members, command& context);

    // Warnings

    void warnings(const guild_member& member, const snowflake& channel_id);
    void warnings(const vector<guild_member>& members, const snowflake& channel_id);

    // Mute

    void mute(const guild_member& member, command& context);
    void mute(const vector<guild_member>& members, command& context);

    // Unmute

    void unmute(const guild_member& member, command& context);
    void unmute(const vector<guild_member>& members, command& context);

    // Delete warning

    void delwarn(const warn_t& warn_id, const snowflake& channel_id, const string& reason);
    void delwarn(const vector<warn_t>& warn_ids, command& context);

    // Clear warnings

    void clear_warns(const guild_member& member, const string& reason);
    void clear_warns(const vector<guild_member>& members, const string& reason);

    // Unban Hard

    void unban_hard(const variant<guild_member, user>& user_or_member, command& context);
    void unban_hard(const vector<variant<guild_member, user>>& users_or_members, command& context);

    // Unban

    void unban(const variant<guild_member, user>& user_or_member, command& context);
    void unban(const vector<variant<guild_member, user>>& users_or_members, command& context);

    // Lock

    void lock(const snowflake& target_channel_id, command& context);
    void lock(const vector<snowflake>& channels_id, command& context);

    // Unlock

    void unlock(const snowflake& target_channel_id, command& context);
    void unlock(const vector<snowflake>& channels_id, command& context);
    // Lockdown

    void lockdown(const snowflake& channel_id, command& context);

    // End lockdown

    void end_lockdown(const snowflake& channel_id, const string& reason);

};


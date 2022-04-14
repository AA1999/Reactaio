//
// Created by arshia on 4/12/22.
//

#include "moderation.h"
#include "../../exceptions/missing_permissions.h"
#include "../../exceptions/owner_exception.h"

using dpp::find_guild;
using dpp::find_role;
using dpp::find_user;
using dpp::p_administrator;
using dpp::p_ban_members;
using dpp::p_kick_members;
using dpp::p_mute_members;
using dpp::p_manage_messages;

void moderation::check_perms(const guild_member& author, const guild_member& target, const snowflake& channel_id) {
    if(author.user_id == target.user_id)
        throw missing_permissions{"Self-moderation is not permitted."};
    auto guild{find_guild(author.guild_id)};
    if(target.user_id == guild->owner_id)
        throw owner_exception{};
    auto author_top = find_role(author.roles.at(author.roles.size() - 1));
    auto target_top = find_role(target.roles.at(target.roles.size() - 1));
    if(author_top->position <= target_top->position)
        throw missing_permissions{"Cannot moderate user with higher top role than you."};
}

void moderation::kick_perms_check(const guild_member& author, const snowflake& channel_id) {
	auto guild = find_guild(author.guild_id);
	auto author_user = find_user(author.user_id);
	
	auto author_perms = guild->base_permissions(author_user);
    if(!(author_perms & p_kick_members))
        throw missing_permissions{"You do not have permission to kick members."};
}

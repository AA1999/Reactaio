//
// Created by arshia on 2/24/23.
//

#pragma once

#include <dpp/dpp.h>

#include "../core/containers/fifo_map.h"


extern inline const reactaio::internal::fifo_map<std::string, dpp::audit_type> audit_log_events{
		{"guild_update", dpp::audit_type::aut_guild_update},
		{"channel_create", dpp::audit_type::aut_channel_create},
		{"channel_update", dpp::audit_type::aut_channel_update},
		{"channel_delete", dpp::audit_type::aut_channel_delete},
		{"channel_overwrite_create", dpp::audit_type::aut_channel_overwrite_create},
		{"channel_overwrite_update", dpp::audit_type::aut_channel_overwrite_update},
		{"member_kick", dpp::audit_type::aut_member_kick},
		{"member_prune", dpp::audit_type::aut_member_prune},
		{"member_ban_add", dpp::audit_type::aut_member_ban_add},
		{"member_ban_remove", dpp::audit_type::aut_member_ban_remove},
		{"member_update", dpp::audit_type::aut_member_update},
		{"member_role_update", dpp::audit_type::aut_member_role_update},
		{"member_move", dpp::audit_type::aut_member_move},
		{"member_disconnect", dpp::audit_type::aut_member_disconnect},
		{"bot_add", dpp::audit_type::aut_bot_add},
		{"role_create", dpp::audit_type::aut_role_create},
		{"role_update", dpp::audit_type::aut_role_update},
		{"role_delete", dpp::audit_type::aut_role_delete},
		{"invite_create", dpp::audit_type::aut_invite_create},
		{"invite_update", dpp::audit_type::aut_invite_update},
		{"invite_delete", dpp::audit_type::aut_invite_delete},
		{"webhook_create", dpp::audit_type::aut_webhook_create},
		{"webhook_update", dpp::audit_type::aut_webhook_update},
		{"webhook_delete", dpp::audit_type::aut_webhook_delete},
		{"emoji_create", dpp::audit_type::aut_emoji_create},
		{"emoji_update", dpp::audit_type::aut_emoji_update},
		{"emoji_delete", dpp::audit_type::aut_emoji_delete},
		{"message_delete", dpp::audit_type::aut_message_delete},
		{"message_bulk_delete", dpp::audit_type::aut_message_bulk_delete},
		{"message_pin", dpp::audit_type::aut_message_pin},
		{"message_unpin", dpp::audit_type::aut_message_unpin},
		{"integration_create", dpp::audit_type::aut_integration_create},
		{"integration_update", dpp::audit_type::aut_integration_update},
		{"integration_delete", dpp::audit_type::aut_integration_delete},
		{"stage_instance_create", dpp::audit_type::aut_stage_instance_create},
		{"stage_instance_update", dpp::audit_type::aut_stage_instance_update},
		{"stage_instance_delete", dpp::audit_type::aut_stage_instance_delete},
		{"sticker_create", dpp::audit_type::aut_sticker_create},
		{"sticker_update", dpp::audit_type::aut_sticker_update},
		{"sticker_delete", dpp::audit_type::aut_sticker_update},
		{"guild_scheduled_event_create", dpp::audit_type::aut_guild_scheduled_event_create},
		{"guild_scheduled_event_update", dpp::audit_type::aut_guild_scheduled_event_update},
		{"guild_scheduled_event_delete", dpp::audit_type::aut_guild_scheduled_event_delete},
		{"thread_create", dpp::audit_type::aut_thread_create},
		{"thread_update", dpp::audit_type::aut_thread_update},
		{"thread_delete", dpp::audit_type::aut_thread_delete},
		{"appcommand_permission_update", dpp::audit_type::aut_appcommand_permission_update},
		{"automod_rule_create", dpp::audit_type::aut_automod_rule_create},
		{"automod_rule_update", dpp::audit_type::aut_automod_rule_update},
		{"automod_rule_delete", dpp::audit_type::aut_automod_rule_delete},
		{"automod_block_message", dpp::audit_type::aut_automod_block_message},
		{"automod_user_communication_disabled", dpp::audit_type::aut_automod_user_communication_disabled },
		{"creator_monetization_request_created", dpp::audit_type::aut_creator_monetization_request_created},
		{"creator_monetization_terms_accepted", dpp::audit_type::aut_creator_monetization_terms_accepted}
};

#define NAME_OF(variable) ((decltype(&(variable)))nullptr, #variable)

template<>
struct std::formatter<dpp::audit_type>: std::formatter<std::string> {
	auto format(dpp::audit_type const& type, format_context& context) const {
		return formatter<string>::format(std::format("{}", NAME_OF(type)), context);
	}
};
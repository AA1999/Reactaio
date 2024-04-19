//
// Created by arshia on 2/29/24.
//

#include "view_muted_members.h"
#include "../../../core/consts.h"
#include "../../../core/colors.h"
#include "../../../core/helpers.h"
#include "../../../core/datatypes/message_paginator.h"

#include <algorithm>
#include <execution>

void view_muted_members::wrapper_function() {
	check_permissions();
	if(!cancel_operation)
		process_response();
}

void view_muted_members::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = dpp::find_guild_member(command.guild->id , bot_user.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();

	if(!bot_top_role->has_manage_guild()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Manage Guild permission.");
	}
}

void view_muted_members::process_response() {
	if(are_all_errors()) {
		auto split_format = join_with_limit(errors, bot_max_embed_chars);
		if(split_format.size() == 1) {
			response = dpp::message{command.channel_id, split_format.front()}.set_flags(dpp::m_ephemeral);
			if(command.interaction) { // Will always be true but failsafe
				(*command.interaction)->edit_response(response);
				return;
			}
		}
		else {
			response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
			message_paginator paginator{response, errors, command};
			paginator.start();
		}
		return;
	}
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	pqxx::work transaction{*command.connection};
	auto const result = transaction.exec_prepared1("get_mute_role", std::to_string(command.guild->id));
	transaction.commit();
	std::vector<std::string> members;
	auto const time_now = std::time(nullptr);
	auto const embed_template = dpp::embed()
						 .set_color(color::INFO_COLOR)
						 .set_timestamp(time_now)
						 .set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));
	if(!result["mute_role"].is_null()) {
		auto const mute_id = result["mute_role"].as<snowflake_t>();
		auto const mute_role = std::make_shared<dpp::role>(*dpp::find_role(mute_id));
		std::for_each(std::execution::par_unseq, command.guild->members.begin(), command.guild->members.end(), [&](std::pair<dpp::snowflake, dpp::guild_member> const& pair) {
			auto const& [snowflake, member] = pair;
			auto member_roles = get_roles_sorted(member);
			if(contains(member_roles, mute_role)) {
				auto const query = transaction.exec_prepared1("view_tempmute", std::to_string(member.user_id), std::to_string(command.guild->id));
				transaction.commit();
				if(query["user_id"].is_null())
					return;
				auto const start_date_str = query["start_date"].as<std::string>();
				auto const end_date_str = query["end_date"].as<std::string>();
				auto const tempmute_id = query["mute_id"].as<snowflake_t>();
				auto const start_date = parse_psql_timestamp(start_date_str, iso_format);
				auto const end_date = parse_psql_timestamp(end_date_str, iso_format);
				auto const mod_id = query["mod_id"].as<snowflake_t>();
				auto const mod = dpp::find_guild_member(command.guild->id, mod_id);
				auto const reason = query["reason"].as<std::string>();
				auto embed{embed_template};
				embed.set_title(std::format("Mute id: {}", tempmute_id));
				embed.add_field("Member: ", member.get_mention(), true);
				embed.add_field("Muted by: ", mod.get_mention(), true);
				embed.add_field("Start: ", dpp::utility::timestamp(start_date.time_since_epoch().count(), dpp::utility::tf_relative_time));
				embed.add_field("Muted Until: ", dpp::utility::timestamp(end_date.time_since_epoch().count(), dpp::utility::tf_relative_time), true);
				embed.add_field("Reason: ", reason, true);
				response.add_embed(embed);
			}
		});
	}

	std::for_each(std::execution::par_unseq, command.guild->members.begin(), command.guild->members.end(), [&](std::pair<dpp::snowflake, dpp::guild_member> const& pair) {
		auto const& [snowflake, member] = pair;
		if(member.is_communication_disabled()) {
			auto const end = member.communication_disabled_until;
			auto embed{embed_template};
			auto const future = dpp::utility::timestamp(end, dpp::utility::tf_relative_time);
			embed.set_title("Timeout: ");
			embed.add_field("Member: ", member.get_mention(), true);
			embed.add_field("Timeout until: ", future, true);
			response.add_embed(embed);
		}
	});

	if(response.embeds.size() > bot_max_embeds) {
		message_paginator paginator{response, command};
		paginator.start();
		return;
	}

	if(command.interaction) {
		(*command.interaction)->edit_response(response);
		return;;
	}
	if(command.interaction) // Will always be true but failsafe.
		(*command.interaction)->edit_response("This guild has no muted/timed out members."); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

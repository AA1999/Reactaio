//
// Created by arshia on 5/21/24.
//

#include "change_duration.h"

#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"
#include "../../mod_action.h"


void change_duration::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void change_duration::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = dpp::find_guild_member(command.guild->id, bot_user.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const &bot_top_role = bot_roles.front();

	if (!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Moderate Members permission.");
	}

	//TODO check if author has permission to do this by either specified role or Moderate Members perm
}

void change_duration::update_duration() {
	pqxx::work transaction{*command.connection};
	auto const modcase_lookup_query = transaction.exec_prepared1("modcase_view", case_id, std::to_string(command.guild->id));
	if(modcase_lookup_query["mod_id"].is_null()) {
		errors.emplace_back("❌ Invalid case id.");
		return;
	}
	auto action_type = modcase_lookup_query["action"].as<std::string_view>();
	auto punished_id = modcase_lookup_query["punished_id"].as<snowflake_t>();
	if(!contains(actions_with_duration, action_type)) {
		errors.emplace_back("❌ This action cannot have a duration. Only bans, timeouts and mutes have this.");
		return;
	}
	auto duration_str = modcase_lookup_query["duration"].as<std::string>();
	internal::duration const duration{duration_str};
	if(action_type == internal::mod_action_name::BAN) {
		auto get_ban_query = transaction.exec_prepared1("get_ban", std::to_string(punished_id), std::to_string(command.guild->id));
		transaction.commit();
		if(get_ban_query["ban_id"].is_null()) {
			errors.emplace_back("❌ Ban not found. It's possible that the user is already unbanned.");
			return;
		}
		auto const ban_id = get_ban_query["ban_id"].as<snowflake_t>();
		auto const start = parse_psql_timestamp(get_ban_query["start_date"].as<std::string>(), iso_format);
		auto const new_end = start + new_duration.to_seconds();
		auto const new_end_str = std::format(iso_format, new_end);
		transaction.exec_prepared0("update_tempban", new_end_str, ban_id);
		transaction.commit();
	}
	else if (action_type == internal::mod_action_name::MUTE) {
		auto get_mute_query = transaction.exec_prepared1("get_tempmute", std::to_string(punished_id), std::to_string(command.guild->id));
		transaction.commit();
		if(!get_mute_query["mute_id"].is_null()) {
			errors.emplace_back("❌ Mute not found. It's possible that it's");
			return;
		}
		auto const mute_id = get_mute_query["mute_id"].as<snowflake_t>();
		auto const start = parse_psql_timestamp(get_mute_query["start_date"].as<std::string>(), iso_format);
		auto const new_end = start + new_duration.to_seconds();
		auto const new_end_str = std::format(iso_format, new_end);
		transaction.exec_prepared0("update_tempmute", new_end_str, mute_id);
	}
	else {
		auto const punished_member = dpp::find_guild_member(command.guild->id, punished_id);
		if(!punished_member.is_communication_disabled()) {
			errors.emplace_back("❌ Member isn't timed out anymore.");
			return;
		}
		if(duration.seconds() > max_timeout_seconds) {
			errors.emplace_back("❌ Max duration for a timeout is 28 days.");
			return;
		}
		auto const now = std::chrono::system_clock::now();
		auto const future = now + duration.to_seconds();
		command.bot->guild_member_timeout(command.guild->id, punished_id, future.time_since_epoch().count(), [this](dpp::confirmation_callback_t const& completion) {
			if(completion.is_error()) {
				auto const error = completion.get_error();
				errors.push_back(std::format("❌ Error {}: {}", error.code, error.human_readable));
			}
		});
	}
}

void change_duration::process_response() {
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
	pqxx::work transaction{*command.connection};

	auto const result = transaction.exec_prepared1("modcase_update_reason", std::string{command.reason}, case_id, std::to_string(command.guild->id));
	transaction.commit();
	if(!result["case_id"].is_null()) {
		response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
		auto const time_now = std::time(nullptr);
		auto embed = dpp::embed()
							 .set_color(color::INFO_COLOR)
							 .set_title(std::format("Case {} Duration updated", case_id))
							 .set_timestamp(time_now)
							 .set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));
		embed.add_field("Old Duration: ",  std::string{command.duration});
		embed.add_field("New Duration: ",  new_duration.to_string(), true);
		response.add_embed(embed);
		if(command.interaction) {
			(*command.interaction)->edit_response(response);
			return;
		}
	}
	transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author->user_id),
								  reactaio::internal::mod_action_name::DURATION, dpp::utility::current_date_time());
	transaction.commit();
	if(command.interaction) // Will always be true but failsafe.
		(*command.interaction)->edit_response(std::format("Modcase {} does not exist in the guild", case_id)); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

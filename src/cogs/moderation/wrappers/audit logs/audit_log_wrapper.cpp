//
// Created by arshia on 3/4/24.
//

#include <execution>

#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../audit_log_actions.h"
#include "audit_log_wrapper.h"

#include <memory>


void audit_log_wrapper::wrapper_function() {
	check_permissions();
	if(!cancel_operation)
		process_response();
}

void audit_log_wrapper::check_permissions() {
	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();
	auto const author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.begin();

	if (!bot_top_role->has_view_audit_log()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot doesn't have the appropriate permissions. Please make sure the View Audit Log permission is enabled.");
	}

	//TODO Check for permission to either Ban Members in discord or a specified role in bot (adding to db soon)
}

void audit_log_wrapper::recursive_call(dpp::snowflake after) {
	command.bot->guild_auditlog_get(command.guild->id, 0, 0, 0, after, max_guild_audit_log_fetch, [this, after](dpp::confirmation_callback_t const& callback) {
		if(callback.is_error()) {
			auto const error = callback.get_error();
			errors.push_back(std::format("Error {}: {}", error.code, error.human_readable));
			return;
		}
		auto audit_map = callback.get<dpp::auditlog>();
		auto new_after{after};
		for(auto const& entry: audit_map.entries) {
			audit_entries_.insert(std::make_shared<dpp::audit_entry>(entry));
			if(entry.user_id > new_after)
				new_after = entry.user_id;
		}
		recursive_call(new_after);
	});
}

void audit_log_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
								  .set_title("Error while fetching guild audit log: ")
								  .set_color(ERROR_COLOR)
								  .set_timestamp(time_now);
		if(format_split.size() == 1) {
			base_embed.set_description(format_split[0]);
			message.add_embed(base_embed);
		}
		else {
			for (auto const& error : format_split) {
				auto embed{base_embed};
				embed.set_description(error);
				message.add_embed(embed);
			}
		}
		if(command.interaction) { // This is always true but a failsafe
			if(are_all_errors())
				message.set_flags(dpp::m_ephemeral); // Invisible error message.
			if(format_split.size() == 1)
				(*command.interaction)->edit_response(message);
			else {
				message_paginator paginator{message, command};
				paginator.start();
			}
		}
		return;
	}

	message.set_flags(dpp::m_ephemeral);

	std::vector<std::string> audit_logs;
	for(auto const& entry: audit_entries_ ) {
		auto const user_id = entry->user_id;
		auto type_string = audit_log_events.find_key(entry->type);
		replace_all(type_string, "_", " ");
		auto const audit_type = to_titlecase(type_string);
		auto const target_id = entry->user_id;
		auto const audit_changes = entry->changes;
		std::vector<std::string> changes_vector;
		std::ranges::transform(audit_changes, std::back_inserter(changes_vector), [](dpp::audit_change const& change) {
			return std::format("changed property {} from {} to {}.", change.key, change.old_value, change.new_value);
		});
		const std::string changes = join(changes_vector, ", ");
		auto const member = find_guild_member(command.guild->id, user_id);
		audit_logs.push_back(std::format("Action: {} done by {} to {} {}", audit_type, member.get_mention(), std::to_string(target_id), changes));
	}
	auto const time_now = std::time(nullptr);

	auto base_embed = dpp::embed()
							  .set_title("Error while fetching guild audit logs: ")
							  .set_color(INFO_COLOR)
							  .set_timestamp(time_now);
	auto const format_split = join_with_limit(audit_logs, bot_max_embed_chars);
	if(format_split.size() == 1) {
		base_embed.set_description(format_split.at(0));
		message.add_embed(base_embed);
		if(command.interaction) {
			(*command.interaction)->edit_response(message);
			return;
		}
	}

	for(auto const& ban: format_split) {
		auto embed{base_embed};
		embed.set_description(ban);
		message.add_embed(embed);
	}
	if(command.interaction) {
		message_paginator paginator{message, command};
		paginator.start();
	}
}
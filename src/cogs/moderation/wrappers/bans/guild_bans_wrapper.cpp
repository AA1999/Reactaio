//
// Created by arshia on 11/6/23.
//

#include <format>

#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"
#include "guild_bans_wrapper.h"
#include "../../mod_action.h"

void guild_bans_wrapper::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	recursive_call();
	process_response();
}

void guild_bans_wrapper::check_permissions() {
	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();
	auto const author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	if(!bot_top_role->has_ban_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot doesn't have the appropriate permissions. Please make sure the Ban Members permission is enabled.");
	}

	auto const roles = get_permitted_roles(internal::mod_action_name::VIEW_BAN_LIST);

	if((roles.empty() && !author_top_role->has_ban_members() || (!roles.empty() && !author_top_role->has_ban_members() && !roles.contains(author_top_role)))) {
		cancel_operation = true;
		errors.emplace_back("❌ You do not have permission to run this command.");
	}

	if(cancel_operation) {
		auto const organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while banning member(s): ")
								  .set_color(ERROR_COLOR)
								  .set_timestamp(time_now);
		if (organized_errors.size() == 1) {
			base_embed.set_description(organized_errors[0]);
			error_message.add_embed(base_embed);
		}
		else {
			for (auto const &error: organized_errors) {
				auto embed{base_embed};
				embed.set_description(error);
				error_message.add_embed(embed);
			}
		}
		if(command.interaction) {
			error_message.set_flags(dpp::m_ephemeral);
			if(organized_errors.size() == 1)
				(*command.interaction)->edit_response(error_message);
			else {
				message_paginator paginator{error_message, command};
				paginator.start();
			}
		}
		else
			invoke_error_webhook();
	}

}

void guild_bans_wrapper::recursive_call(dpp::snowflake after) {
	command.bot->guild_get_bans(command.guild->id, 0, after, max_guild_ban_fetch, [this, after](dpp::confirmation_callback_t const& completion){
		if(completion.is_error()) {
			auto error = completion.get_error();
			errors.push_back(std::format("❌ Error {}: {}", error.code, error.human_readable));
		}
		else {
			auto event_map = completion.get<dpp::ban_map>();
			dpp::snowflake new_after{after};

			for(auto const& [user_id, ban]: event_map) {
				if(new_after < user_id)
					new_after = user_id;
				bans.insert(&ban);
			}

			if(event_map.size() < max_guild_ban_fetch) // All bans are fetched.
				return;
			recursive_call(new_after);
		}
	});
}

void guild_bans_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	if (has_error()) {
		auto const format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);

		auto base_embed	= dpp::embed()
								.set_title("Error while fetching guild bans: ")
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
		else
			invoke_error_webhook();
		return;
	}

	message.set_flags(dpp::m_ephemeral);

	for(auto const& ban: bans) {
		command.bot->user_get_cached(ban->user_id, [this, ban](dpp::confirmation_callback_t const& completion){
			if(!completion.is_error())
				banned_usernames.push_back(std::format("User **{}** Reason: {}", std::get<dpp::user_identified>(completion.value).format_username(),
													   ban->reason));
		});
	}

	auto const format_split = join_with_limit(banned_usernames, bot_max_embed_chars);
	auto const time_now = std::time(nullptr);

	auto base_embed = dpp::embed()
							  .set_title("Error while fetching guild bans: ")
							  .set_color(INFO_COLOR)
							  .set_timestamp(time_now);

	if(command.interaction)
		log_command_invoke(internal::mod_action_name::VIEW_BAN_LIST);

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
	else
		invoke_error_webhook();
}

//
// Created by arshia on 1/22/24.
//

#include "change_reason.h"

#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../mod_action.h"

void change_reason::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void change_reason::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = dpp::find_guild_member(command.guild->id , bot_user.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Moderate Members permission.");
	}

	//TODO check if author has permission to do this by either specified role or Moderate Members perm
}

void change_reason::process_response() {
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
							 .set_title(std::format("Case {} Reason updated", case_id))
							 .set_timestamp(time_now)
							 .set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));
		embed.add_field("New Reason: ", std::string{command.reason});
		response.add_embed(embed);
		if(command.interaction) {
			(*command.interaction)->edit_response(response);
			return;
		}
	}
	transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author->user_id),
								  reactaio::internal::mod_action_name::REASON, dpp::utility::current_date_time());
	transaction.commit();
	if(command.interaction) // Will always be true but failsafe.
		(*command.interaction)->edit_response(std::format("Modcase {} does not exist in the guild", case_id)); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

//
// Created by arshia on 1/22/24.
//

#include "view_modcase.h"
#include "../../../core/helpers.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"


void view_modcase::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_response();
}

void view_modcase::check_permissions() {
	auto const bot_user = command.bot->me;
	auto const bot_member = dpp::find_guild_member(command.guild->id , bot_user.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const bot_top_role = bot_roles.front();

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot's top role has Moderate Members permission.");
	}

	//TODO check if author has permission to do this by either specified role or Moderate Members perm
}

void view_modcase::process_response() {
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
	auto result = transaction.exec_prepared1("view_modcase", case_id, std::to_string(command.guild->id));
	transaction.commit();
	response = dpp::message{command.channel_id, ""}.set_flags(dpp::m_ephemeral);
	if(!result["punished_id"].is_null()) {
		dpp::snowflake user_id{result["punished_id"].as<snowflake_t>()};
		command.bot->user_get_cached(user_id, [this, user_id, result](dpp::confirmation_callback_t const& completion){
			auto time_now = std::time(nullptr);
			auto embed = dpp::embed()
								 .set_color(color::INFO_COLOR)
								 .set_title(std::format("Case {}", case_id))
								 .set_timestamp(time_now)
								 .set_footer(dpp::embed_footer().set_text(std::format("Guild id {}", std::to_string(command.guild->id))));
			if(!completion.is_error()) {
				auto username = std::get<dpp::user_identified>(completion.value).format_username();
				auto punishment_type = result["action"].as<std::string>();
				embed.add_field(std::vformat("User: {}", std::make_format_args(username)), std::vformat("Punishment type: {}", std::make_format_args(punishment_type)), true);
				auto reason = result["reason"].as<std::string>();
				embed.add_field("Reason: ", reason, true);
				if(!result["duration"].is_null()) {
					auto duration_str = result["duration"].as<std::string>();
					embed.add_field("Duration: ", duration_str);
				}
			}
			else {
				embed.add_field(std::format("User id: {}", user_id.str()), "");
			}
			response.add_embed(embed);
		});
		if(command.interaction) { // Will always be true but failsafe
			(*command.interaction)->edit_response(response);
			return;
		}
	}
	if(command.interaction) // Will always be true but failsafe.
		(*command.interaction)->edit_response(std::format("Modcase {} does not exist in the guild", case_id)); // This is a rather unlikely scenario but there might be a mod wanting to test this
}

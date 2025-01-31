//
// Created by arshia on 3/18/24.
//

#include "unlock_wrapper.h"
#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../mod_action.h"


void unlock_wrapper::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_unlocks();
	process_response();
}

void unlock_wrapper::check_permissions() {
	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();
	auto const author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	if(!bot_top_role->has_manage_roles()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check ifthe bot has Manage Roles permission.");
	}


	auto const roles = get_permitted_roles(internal::mod_action_name::UNLOCK);

	if((roles.empty() && !author_top_role->has_manage_roles() || (!roles.empty() && !author_top_role->has_manage_roles() && !roles.contains(author_top_role)))) {
		cancel_operation = true;
		errors.emplace_back("❌ You do not have permission to run this command.");
	}
	if(cancel_operation) {
		auto const organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while unlocking channel(s): ")
								  .set_color(ERROR_COLOR)
								  .set_timestamp(time_now);
		if(organized_errors.size() == 1) {
			base_embed.set_description(organized_errors[0]);
			error_message.add_embed(base_embed);
		}
		else {
			for(auto const& error: organized_errors) {
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

void unlock_wrapper::process_unlocks() {
	if(channels.empty())
		channels.insert(find_channel(command.channel_id));
	for(auto const& channel: channels) {
		command.bot->set_audit_reason(std::format("Unlocked by {} for reason: {}.", command.author->get_user()->format_username(), command.reason)).channel_edit_permissions(*channel, command.guild->id, dpp::permissions::p_send_messages, 0,  false,[channel, this](const dpp::confirmation_callback_t& completion) {
			lambda_callback(completion, channel);
		});
	}
}

void unlock_wrapper::lambda_callback(dpp::confirmation_callback_t const& completion, channel_ptr const& channel) {
	if(completion.is_error()) {
		auto const error = completion.get_error();
		errors.push_back(std::format("Unable to unlock channel {}. Error Code {}: {}", channel->get_mention(), error.code, error.human_readable));
		return;
	}
	auto transaction = pqxx::work{*command.connection};
	auto const max_query = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
	auto const max_id = std::get<0>(max_query.as<case_t>()) + 1;
	transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id, internal::mod_action_name::UNLOCK,
							  std::to_string(command.author->user_id), std::to_string(channel->id), command.reason);
	transaction.commit();
}

void unlock_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author->get_user();

	if(has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
								  .set_title("Error while soft unlocking channel(s): ")
								  .set_color(ERROR_COLOR)
								  .set_timestamp(time_now);
		if(format_split.size() == 1) {
			base_embed.set_description(format_split[0]);
			message.add_embed(base_embed);
		}
		else {
			for(auto const& error : format_split) {
				auto embed{base_embed};
				embed.set_description(error);
				message.add_embed(embed);
			}
		}
		if(command.interaction) {
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
	}
	if(!are_all_errors()) {
		std::vector<std::string> unlocked_mentions;
		shared_vector<dpp::channel> unlocked_channels;

		reactaio::set_difference(channels, channels_with_errors, unlocked_channels);

		std::ranges::transform(unlocked_channels, std::back_inserter(unlocked_mentions), [](auto const& channel) {
			return std::format("{}", channel->get_mention());
		});

		auto mentions = join(unlocked_mentions, ", ");

		const std::string title{"Unlocked"};
		std::string description;
		std::string gif_url;

		if(channels.size() == 1) {
			description = std::format("{} has been unlocked.", mentions);
			gif_url		= "https://media3.giphy.com/media/LOoaJ2lbqmduxOaZpS/giphy.gif";
		}
		else {
			description = std::format("{} have been unlocked.", mentions);
			gif_url		= "https://i.gifer.com/1Daz.gif";
		}

		auto time_now	= std::time(nullptr);
		auto reason_str = std::string{command.reason};

		auto response = dpp::embed()
								.set_color(RESPONSE_COLOR)
								.set_title(title)
								.set_description(description)
								.set_image(gif_url)
								.set_timestamp(time_now);
		response.add_field("Moderator: ", author_user->get_mention());
		response.add_field("Reason: ", reason_str);

		message.add_embed(response);

		// Modlogs

		auto transaction = pqxx::work{*command.connection};
		auto query = transaction.exec_prepared("channel_modlog", std::to_string(command.guild->id));
		transaction.commit();
		const std::string embed_title = std::format("Channel{} unlocked: ", unlocked_channels.size() == 1 ? "" : "s");
		auto const channel_overwrite_update_webhook_url = query[0]["channel_overwrite_update"];
		if(!channel_overwrite_update_webhook_url.is_null()) {
			auto channel_overwrite_update_webhook = dpp::webhook{channel_overwrite_update_webhook_url.as<std::string>()};

			description = std::format("{} have been unlocked.", mentions);
			time_now = std::time(nullptr);
			auto unlock_log = dpp::embed()
								   .set_color(LOG_COLOR)
								   .set_title(embed_title)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author->get_mention())
								   .add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(unlock_log);

			command.bot->execute_webhook(channel_overwrite_update_webhook, log);
		}
		auto modlog_webhook_url = query[0]["modlog"];
		if(!modlog_webhook_url.is_null()) {
			auto modlog_webhook = dpp::webhook{modlog_webhook_url.as<std::string>()};
			description = std::format("{} have been unlocked.", mentions);

			time_now = std::time(nullptr);
			auto unlock_log = dpp::embed()
								   .set_color(LOG_COLOR)
								   .set_title(embed_title)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author->get_mention())
								   .add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(unlock_log);
			command.bot->execute_webhook(modlog_webhook, log);
		}
		auto public_modlog_webhook_url = query[0]["public_modlog"];
		if(!public_modlog_webhook_url.is_null()) {
			auto public_modlog_webhook = dpp::webhook{public_modlog_webhook_url.as<std::string>()};
			description = std::format("{} have been unlocked.", mentions);
			time_now = std::time(nullptr);
			auto unlock_log = dpp::embed()
								   .set_color(LOG_COLOR)
								   .set_title(embed_title)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author->get_mention())
								   .add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(unlock_log);
			command.bot->execute_webhook(public_modlog_webhook, log);
		}
	}
	if(command.interaction) {
		if(message.embeds.size() == 1)
			(*command.interaction)->edit_response(message);
		else {
			message_paginator paginator{message, command};
			paginator.start();
		}
		log_command_invoke(internal::mod_action_name::UNLOCK);
	}
	else
		command.bot->message_create(message);
}
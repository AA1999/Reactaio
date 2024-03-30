//
// Created by arshia on 3/18/24.
//

#include "unlock_wrapper.h"

#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/datatypes/message_paginator.h"
#include "../../mod_action.h"


void unlock_wrapper::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;
	process_unlocks();
	process_response();
}

void unlock_wrapper::check_permissions() {
	auto const bot_member = dpp::find_guild_member(command.guild->id, command.bot->me.id);

	auto bot_roles = get_roles_sorted(bot_member);
	auto bot_top_role = *bot_roles.begin();

	auto author_roles = get_roles_sorted(command.author);
	auto author_top_role = *author_roles.begin();

	if(!bot_top_role->has_manage_roles()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot has Manage Roles permission.");
	}

	// TODO Check for roles that are allowed to use this command.

	if(cancel_operation) {
		auto messages = join_with_limit(errors, bot_max_embed_chars);
		message_paginator paginator{dpp::message{command.channel_id, ""}, messages, command};
		paginator.start();
	}
}

void unlock_wrapper::process_unlocks() {
	if (channels.empty())
		channels.push_back(*dpp::find_channel(command.channel_id));
	for(auto const& channel: channels) {
		command.bot->set_audit_reason(std::format("Unlocked by {} for reason: {}.", command.author.get_user()->format_username(), command.reason)).channel_edit_permissions(channel, command.guild->id, dpp::permissions::p_send_messages, 0,  false,[channel, this](const dpp::confirmation_callback_t& completion) {
			lambda_callback(completion, channel);
		});
	}
}

void unlock_wrapper::lambda_callback(dpp::confirmation_callback_t const &completion, dpp::channel const &channel) {
	if (completion.is_error()) {
		auto const error = completion.get_error();
		errors.push_back(std::format("Unable to unlock channel {}. Error Code {}: {}", channel.get_mention(), error.code, error.human_readable));
		return;
	}
	auto transaction = pqxx::work{*command.connection};
	auto const max_query = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
	auto const max_id = std::get<0>(max_query.as<case_t>()) + 1;
	transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id, reactaio::internal::mod_action_name::UNLOCK,
							  std::to_string(command.author.user_id), std::to_string(channel.id), command.reason);
	transaction.commit();
}

void unlock_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author.get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
								  .set_title("Error while soft unlocking channel(s): ")
								  .set_color(color::ERROR_COLOR)
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
		if(command.interaction) {
			if(are_all_errors())
				message.set_flags(dpp::m_ephemeral); // Invisible error message.
			if(format_split.size() == 1)
				command.interaction->edit_response(message);
			else {
				message_paginator paginator{message, command};
				paginator.start();
			}
		}
		else {
			// Get bot error webhook
			auto automod_log = dpp::message();
			auto transaction  = pqxx::work{*command.connection};
			auto webhook_url_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			transaction.commit();
			if(webhook_url_query.empty()) { // Bot has no error webhook set.
				automod_log.set_content("This server hasn't set a channel for bot errors. So the errors are being sent to your DMs:");
				for (auto const& error : format_split) {
					auto embed{base_embed};
					embed.set_description(error);
					automod_log.add_embed(embed);
				}
				command.bot->direct_message_create(command.author.user_id, automod_log);
			}
			else {
				auto webhook_url = webhook_url_query[0]["bot_error_logs"].as<std::string>();
				dpp::webhook automod_webhook{webhook_url};
				for (auto const& error : format_split) {
					auto embed{base_embed};
					embed.set_description(error);
					automod_log.add_embed(embed);
				}
				command.bot->execute_webhook(automod_webhook, automod_log);
			}

		}
	}
	if (!are_all_errors()) {
		std::vector<std::string> unlocked_mentions;
		shared_vector<dpp::channel> unlocked_channels;

		filter(unlocked_channels);

		std::ranges::transform(unlocked_channels, std::back_inserter(unlocked_mentions), [](auto const& channel) {
			return std::format("{}", channel->get_mention());
		});

		auto mentions = join(unlocked_mentions, ", ");

		const std::string title{"Locked"};
		std::string description;
		std::string gif_url;

		if (channels.size() == 1) {
			description = std::format("{} has been locked.", mentions);
			gif_url		= "https://media3.giphy.com/media/LOoaJ2lbqmduxOaZpS/giphy.gif";
		}
		else {
			description = std::format("{} have been unlocked.", mentions);
			gif_url		= "https://i.gifer.com/1Daz.gif";
		}

		auto time_now	= std::time(nullptr);
		auto reason_str = std::string{command.reason};

		auto response = dpp::embed()
								.set_color(color::RESPONSE_COLOR)
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
								   .set_color(color::LOG_COLOR)
								   .set_title(embed_title)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author.get_mention())
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
								   .set_color(color::LOG_COLOR)
								   .set_title(embed_title)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author.get_mention())
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
								   .set_color(color::LOG_COLOR)
								   .set_title(embed_title)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author.get_mention())
								   .add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(unlock_log);
			command.bot->execute_webhook(public_modlog_webhook, log);
		}
	}
	if (command.interaction) {
		if(message.embeds.size() == 1)
			command.interaction->edit_response(message);
		else {
			message_paginator paginator{message, command};
			paginator.start();
		}
		// Log command call
		pqxx::work transaction{*command.connection};
		transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author.user_id),
								  reactaio::internal::mod_action_name::UNLOCK, dpp::utility::current_date_time());
		transaction.commit();
	}
	else
		command.bot->message_create(message);
}
//
// Created by arshia on 1/17/24.
//

#include "unmute_wrapper.h"

#include "../../mod_action.h"

#include "../../../core/consts.h"
#include "../../../core/colors.h"
#include "../../../core/helpers.h"
#include "../../../core/datatypes/message_paginator.h"


void unmute_wrapper::wrapper_function() {
	check_permissions();
	if(cancel_operation)
		return;

	process_unmutes();
	process_response();
}

void unmute_wrapper::check_permissions() {
	auto const bot_member = dpp::find_guild_member(command.guild->id, command.bot->me.id);

	auto bot_roles = get_roles_sorted(bot_member);
	auto bot_top_role = *bot_roles.begin();

	auto author_roles = get_roles_sorted(command.author);
	auto author_top_role = *author_roles.begin();

	bool is_owner{false};

	pqxx::work transaction{*command.connection};
	auto protected_roles_query = transaction.exec_prepared("protected_roles", std::to_string(command.guild->id));
	transaction.commit();

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check if the bot has Moderate Members permission.");
	}

	std::vector<dpp::role*> protected_roles;

	if(!protected_roles_query.empty()) {
		auto protected_roles_field = protected_roles_query[0]["protected_roles"];
		auto protected_role_snowflakes = parse_psql_array<dpp::snowflake>(protected_roles_field);
		std::ranges::transform(protected_role_snowflakes.begin(), protected_role_snowflakes.end(),
							   std::back_inserter(protected_roles), [](const dpp::snowflake role_id){
								   return dpp::find_role(role_id);
							   });
	}


	for(auto const& member: members) {
		auto member_roles = get_roles_sorted(member);
		auto member_top_role = *member_roles.begin();

		if(command.author.user_id == member.user_id) { // If for some reason you decided to unmute yourself lol
			if(member.user_id == command.guild->owner_id) { // If you're also the server owner
				errors.emplace_back("❌ Why are you unmuting yourself, server owner? lmfao");
				is_owner = true;
			}
			else {
				errors.emplace_back("❌ You can't unmute yourself lmao.");
			}
			cancel_operation = true;
		}

		if(!is_owner && member.user_id == command.guild->owner_id) { // Unmuting the server owner lmfao
			errors.emplace_back("❌ You can't unmute the server owner lmfao.");
			cancel_operation = true;
		}


		if(command.bot->me.id == member.user_id) { // If you decided to unmute the bot (ReactAIO)
			errors.emplace_back("❌ Can't unmute myself lmfao.");
			cancel_operation = true;
		}

		if(!protected_roles.empty()) {

			std::vector<dpp::role*> member_protected_roles;
			std::ranges::set_intersection(protected_roles.begin(), protected_roles.end(), member_roles.begin(),
										  member_roles.end(), std::back_inserter(member_protected_roles));

			if(!member_protected_roles.empty()) { // If member has any of the protected roles.
				cancel_operation = true;
				std::vector<std::string> role_mentions;
				std::ranges::transform(member_protected_roles.begin(), member_protected_roles.end(), std::back_inserter
									   (role_mentions), [](dpp::role* role){
										   return role->get_mention();
									   });
				std::string role_mentions_str = join(role_mentions, " , ");
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot unmute.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to unmute. Please "
										 "move the bot role above the members and below your staff roles.",
										 member.get_mention()));
			cancel_operation = true;
		}

		if(member_top_role->position > author_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than you do. You can't unmute them.",
										 member.get_mention()));
			cancel_operation = true;
		}
	}

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while unmuting member(s): ")
								  .set_color(color::ERROR_COLOR)
								  .set_timestamp(time_now);
		for (auto const &error: organized_errors) {
			auto embed{base_embed};
			embed.set_description(error);
			error_message.add_embed(embed);
		}
		if(command.interaction) {
			error_message.set_flags(dpp::m_ephemeral);
			if(organized_errors.size() == 1)
				command.interaction->edit_response(error_message);
			else {
				message_paginator paginator{error_message, command};
				paginator.start();
			}
		}
		else {
			auto webhook_url_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			transaction.commit();
			auto bot_error_webhook_url_field = webhook_url_query[0]["bot_error_logs"];
			if(!bot_error_webhook_url_field.is_null()) {
				auto bot_error_webhook_url = bot_error_webhook_url_field.as<std::string>();
				dpp::webhook bot_error_webhook{bot_error_webhook_url};
				command.bot->execute_webhook(bot_error_webhook, error_message);
			}
			else {
				error_message.set_content("This server hasn't set a channel for bot errors. So the errors are being "
										  "sent to your DMs:");
				command.bot->direct_message_create(command.author.user_id, error_message);
			}
		}
	}
}

void unmute_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, const dpp::guild_member &member) {
	if (completion.is_error()) {
		auto error = completion.get_error();
		members_with_errors.push_back(member);
		errors.push_back(std::format("❌ Error code {}: {}", error.code, error.human_readable));
	} else {
		std::string dm_message;
		if(use_mute_callback)
			dm_message = std::format("Your timeout has been removed in {} by {}. Reason: {}",
												 command.guild->name, command.author.get_user()->format_username(), command.reason);
		else
			dm_message = std::format("Your have been unmuted in {} by {}. Reason: {}",
									 command.guild->name, command.author.get_user()->format_username(), command.reason);
		command.bot->direct_message_create(member.user_id, dpp::message{dm_message});
	}
}

void unmute_wrapper::process_unmutes() {
	pqxx::work transaction{*command.connection};
	auto mute_role_id_query = transaction.exec_prepared1("get_mute_role", std::to_string(command.guild->id));
	transaction.commit();
	dpp::role* mute_role;
	if(!mute_role_id_query["mute_role"].is_null()) {
		auto mute_role_id = mute_role_id_query["mute_role"].as<snowflake_t>();
		mute_role = dpp::find_role(mute_role_id);
	}
	else
		mute_role = nullptr;
	use_mute_callback = mute_role != nullptr;
	for(auto const& member: members) {
		if(mute_role != nullptr) {
			command.bot->set_audit_reason(std::format("Unmuted by: {} for reason: {}", command.author.get_user()->format_username(), command.reason)).guild_member_delete_role(command.guild->id, member.user_id, mute_role->id, [this, member](auto const& completion){
				lambda_callback(completion, member);
			});
		}
		command.bot->set_audit_reason(std::format("Timeout removed by: {} for reason: {}", command.author.get_user()->format_username(), command.reason)).guild_member_timeout_remove(command.guild->id, member.user_id, [this, member](dpp::confirmation_callback_t callback){
			lambda_callback(callback, member);
		});
	}
}
void unmute_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author.get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
								  .set_title("Error while unmuting member(s): ")
								  .set_color(color::ERROR_COLOR)
								  .set_timestamp(time_now);
		if (command.interaction) {
			if(format_split.size() == 1) {
				base_embed.set_description(format_split[0]);
				message.add_embed(base_embed);
			}
			else {
				for (auto const &error: format_split) {
					auto embed{base_embed};
					embed.set_description(error);
					message.add_embed(embed);
				}
			}
			if (are_all_errors())
				message.set_flags(dpp::m_ephemeral);// If there's no successful mute, no reason to show the errors publicly.
		}
		else {
			// Get bot error webhook
			auto automod_log = dpp::message{};
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
		auto unmuted_members = std::vector<dpp::guild_member>{};
		auto unmuted_usernames = std::vector<std::string>{};
		auto unmuted_mentions = std::vector<std::string>{};

		std::ranges::copy_if(members, std::back_inserter(unmuted_members), [this](dpp::guild_member const& member){
			return !includes(members_with_errors, member);
		});

		std::ranges::transform(unmuted_members, std::back_inserter(unmuted_usernames), [](dpp::guild_member const& member) {
			return std::format("**{}**", member.get_user()->format_username());
		});

		std::ranges::transform(unmuted_members, std::back_inserter(unmuted_mentions), [](dpp::guild_member const& member) {
			return member.get_mention();
		});

		auto usernames = join(unmuted_usernames, ", ");
		auto mentions  = join(unmuted_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if (unmuted_members.size() == 1) {
			title		= "Unmuted";
			description = std::format("{} has been unmuted.", usernames);
			gif_url		= "https://tenor.com/view/neo-mouthshut-matrix-blue-pill-gif-22455602";
		}
		else {
			title		= "Mass unmuted";
			description = std::format("{} have been unmuted.", usernames);
			gif_url		= "https://canary.discord.com/channels/1011029958740684841/1012300461384138842/1019704810267750440";
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
		auto query = transaction.exec_prepared("modlog", std::to_string(command.guild->id));
		transaction.commit();

		auto modlog_webhook_url = query[0]["modlog"];
		if(!modlog_webhook_url.is_null()) {
			auto modlog_webhook = dpp::webhook{modlog_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(unmuted_members.size() > 1)
				embed_title = "Members unmuted: ";
			else {
				embed_title = "Member unmuted: ";
				embed_image_url = unmuted_members.at(0).get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto mute_log = dpp::embed()
									.set_color(color::LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unmuted.", usernames))
									.add_field("Moderator: ", command.author.get_mention())
									.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(mute_log);
			command.bot->execute_webhook(modlog_webhook, log);
		}
		auto public_modlog_webhook_url = query[0]["public_modlog"];
		if(!public_modlog_webhook_url.is_null()) {
			auto public_modlog_webhook = dpp::webhook{public_modlog_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(unmuted_members.size() > 1)
				embed_title = "Members unmuted: ";
			else {
				embed_title = "Member unmuted: ";
				embed_image_url = unmuted_members.at(0).get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto mute_log = dpp::embed()
									.set_color(color::LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been unmute.", usernames))
									.add_field("Moderator: ", command.author.get_mention())
									.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(mute_log);
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
								  reactaio::internal::mod_action_name::UNMUTE, dpp::utility::current_date_time());
		transaction.commit();
	}
	else
		command.bot->message_create(message);
}
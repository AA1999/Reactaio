//
// Created by arshia on 3/4/23.
//

#include "mute_wrapper.h"
#include "../../mod_action.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/helpers.h"
#include "../../../core/datatypes/message_paginator.h"



void mute_wrapper::wrapper_function() {
	check_permissions();

	if(cancel_operation)
		return;

	process_mutes();
	process_response();
}

void mute_wrapper::check_permissions() {
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

		if(command.author.user_id == member.user_id) { // If for some reason you decided to mute yourself lol
			if(member.user_id == command.guild->owner_id) { // If you're also the server owner
				errors.emplace_back("❌ Why are you muting yourself, server owner? lmfao");
				is_owner = true;
			}
			else {
				errors.emplace_back("❌ You can't mute yourself lmao.");
			}
			cancel_operation = true;
		}

		if(!is_owner && member.user_id == command.guild->owner_id) { // Muting the server owner lmfao
			errors.emplace_back("❌ You can't mute the server owner lmfao.");
			cancel_operation = true;
		}


		if(command.bot->me.id == member.user_id) { // If you decided to mute the bot (ReactAIO)
			errors.emplace_back("❌ Can't mute myself lmfao.");
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
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot mute.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to mute. Please "
			                             "move the bot role above the members and below your staff roles.",
			                             member.get_mention()));
			cancel_operation = true;
		}

		if(member_top_role->position > author_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than you do. You can't mute them.",
			                             member.get_mention()));
			cancel_operation = true;
		}
	}

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
		                          .set_title("Error while muting member(s): ")
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

void mute_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, const dpp::guild_member &member) {
	if(duration) {
		if(completion.is_error()) {
			auto error = completion.get_error();
			members_with_errors.push_back(member);
			errors.push_back(std::format("❌ Error code {}: {}", error.code, error.message));
		}
		else {
			auto now = std::chrono::system_clock::now();
			auto time_delta = duration->to_seconds();
			auto future = now + time_delta;
			auto future_ms = future.time_since_epoch().count();
			std::string time_future_relative = dpp::utility::timestamp(future_ms,
																	   dpp::utility::time_format::tf_relative_time);
			std::string dm_message = std::format("You have been timed out in {} by {} until {}. Reason: {}",
												 command.guild->name, command.author.get_user()->format_username(),
												 time_future_relative, command.reason);
			command.bot->direct_message_create(member.user_id, dpp::message{dm_message});
		}
		return;
	}
	if(completion.is_error()) {
		auto error = completion.get_error();
		members_with_errors.push_back(member);
		errors.push_back(std::format("❌ Error code {}: {}", error.code, error.human_readable));
	}
	else {
		auto transaction = pqxx::work{*command.connection};
		auto id_query = transaction.exec_prepared1("timeout_id", std::to_string(command.guild->id));
		ullong timeout_id = 0;
		if(!id_query["timeout_id"].is_null()) {
			timeout_id = id_query["timeout_id"].as<case_t>() + 1;
		}
		auto now = std::chrono::system_clock::now();
		auto time_delta = std::chrono::days{max_timeout_days};
		auto future = now + time_delta;
		auto future_ms = future.time_since_epoch().count();
		transaction.exec_prepared("permanent_timeout", timeout_id, std::to_string(member.user_id),
								  std::to_string(command.guild->id), std::to_string(command.author.user_id),
								  command.reason);
		transaction.commit();
		std::string time_future_relative = dpp::utility::timestamp(future_ms,
																   dpp::utility::time_format::tf_relative_time);
		std::string dm_message = std::format("You have been timed out in {} by {} until {}. Reason: {}",
											 command.guild->name, command.author.get_user()->format_username(),
											 time_future_relative, command.reason);
		command.bot->direct_message_create(member.user_id, dpp::message{dm_message});
	}
}

void mute_wrapper::process_mutes() {
	pqxx::work transaction{*command.connection};
	auto query = transaction.exec_prepared("use_timeout", std::to_string(command.guild->id));
	transaction.commit();
	auto use_timeout_field = query[0]["use_timeout"];
	if(use_timeout_field.is_null())
		use_timeout = true;
	else {
		use_timeout = use_timeout_field.as<bool>();
	}
	duration = parse_human_time(command.duration);
	if(use_timeout) {
		if(duration) {
			if(duration->to_seconds().count() > max_timeout_seconds) {
				errors.emplace_back("❌ Invalid timeout duration. Timeouts can only be max of 28 days.");
			}
			else {
				auto now = std::chrono::system_clock::now();
				auto time_delta = duration->to_seconds();
				auto future = now + time_delta;
				auto future_ms = future.time_since_epoch().count();
				for(auto const& member: members) {
					command.bot->guild_member_timeout(command.guild->id, member.user_id, future_ms, [this, member](auto& completion){
						lambda_callback(completion, member);
					});
				}
			}
		}
		else {
			for(auto const& member: members) {
				command.bot->guild_member_timeout(command.guild->id, member.user_id, max_timeout_seconds, [this, member](auto& completion){
					lambda_callback(completion, member);
				});
			}
		}
	}
	else {
		auto mute_role_id_query = transaction.exec_prepared1("get_mute_role", std::to_string(command.guild->id));
		if(mute_role_id_query["mute_role"].is_null()) {
			errors.emplace_back("❌ No mute role set. Either set one or use the timeout feature.");
			return;
		}
		auto mute_role_id = mute_role_id_query["mute_role"].as<snowflake_t>();
		auto mute_role = dpp::find_role(mute_role_id);
		if(mute_role == nullptr) {
			errors.emplace_back("❌ No mute role set. Either set one or use the timeout feature.");
			return;
		}
		for(auto const& member: members) {
			command.bot->set_audit_reason(std::string{command.reason}).guild_member_add_role(command.guild->id, member.user_id, mute_role_id, [this, member](auto& completion){
				if(completion.is_error()) {
					auto error = completion.get_error();
					errors.push_back(std::format("❌ Error code {}: {}", error.code, error.message));
					members_with_errors.push_back(member);
				}
				else {
					ullong mute_id = 0;
					auto transaction = pqxx::work {*command.connection};
					auto mute_id_row = transaction.exec_prepared1("get_mute_id", std::to_string(command.guild->id));
					if(!mute_id_row["mute_id"].is_null()) {
						mute_id = mute_id_row["mute_id"].as<case_t>() + 1;
					}
					if(duration) {
						auto now = std::chrono::system_clock::now();
						auto now_ms = now.time_since_epoch().count();
						auto time_delta = duration->to_seconds();
						auto future = now + time_delta;
						auto future_ms = future.time_since_epoch().count();
						std::string time_future_relative = dpp::utility::timestamp(future_ms,
																				   dpp::utility::time_format::tf_relative_time);
						std::string dm_message = std::format("You have been muted in {} by {} until {}. Reason: {}",
															 command.guild->name, command.author.get_user()->format_username(),
															 time_future_relative, command.reason);
						transaction.exec_prepared("tempmute", mute_id, std::to_string(member.user_id), std::to_string(command.guild->id), now_ms, future_ms);
						transaction.commit();
						command.bot->direct_message_create(member.user_id, dpp::message{dm_message});
					}
					else {
						std::string dm_message = std::format("You have been muted in {} by {} permanently. Reason: {}",
															 command.guild->name, command.author.get_user()->format_username(),
															 command.reason);
						command.bot->direct_message_create(member.user_id, dpp::message{dm_message});
					}
				}
			});
		}
	}
}

void mute_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author.get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
								  .set_title("Error while muting member(s): ")
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
		auto muted_members = std::vector<dpp::guild_member>{};
		auto muted_usernames = std::vector<std::string>{};
		auto muted_mentions = std::vector<std::string>{};

		std::ranges::copy_if(members, std::back_inserter(muted_members), [this](dpp::guild_member const& member){
			return !includes(members_with_errors, member);
		});

		std::ranges::transform(muted_members, std::back_inserter(muted_usernames), [](dpp::guild_member const& member) {
			return std::format("**{}**", member.get_user()->format_username());
		});

		std::ranges::transform(muted_members, std::back_inserter(muted_mentions), [](dpp::guild_member const& member) {
			return member.get_mention();
		});

		auto usernames = join(muted_usernames, ", ");
		auto mentions  = join(muted_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if (muted_members.size() == 1) {
			title		= "Muted";
			description = std::format("{} has been muted.", usernames);
			gif_url		= "https://tenor.com/view/neo-mouthshut-matrix-blue-pill-gif-22455602";
		}
		else {
			title		= "Mass muted";
			description = std::format("{} have been muted.", usernames);
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
			if(muted_members.size() > 1)
				embed_title = "Members muted: ";
			else {
				embed_title = "Member muted: ";
				embed_image_url = muted_members.at(0).get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto mute_log = dpp::embed()
									.set_color(color::LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been muted.", usernames))
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
			if(muted_members.size() > 1)
				embed_title = "Members muted: ";
			else {
				embed_title = "Member muted: ";
				embed_image_url = muted_members.at(0).get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto mute_log = dpp::embed()
									.set_color(color::LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been muted.", usernames))
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
		auto action_name = use_timeout ? reactaio::internal::mod_action_name::TIMEOUT : reactaio::internal::mod_action_name::MUTE;

		transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author.user_id),
								  action_name, dpp::utility::current_date_time());
		transaction.commit();
	}
	else
		command.bot->message_create(message);
}

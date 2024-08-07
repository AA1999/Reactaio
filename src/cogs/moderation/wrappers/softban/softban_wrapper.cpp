//
// Created by arshia on 1/14/24.
//

#include "softban_wrapper.h"
#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../mod_action.h"


void softban_wrapper::wrapper_function() {
	for(auto& member_or_user: snowflakes) {
		if(auto const* member_pointer = std::get_if<member_ptr>(&member_or_user)) {
			members.insert(*member_pointer);
			users.insert((*member_pointer)->get_user());
		}
		else if(auto const user_pointer = std::get_if<user_ptr>(&member_or_user)) {
			users.insert(*user_pointer);
		}
		else { // Will never happen but failsafe
			invalid_user = true;
			errors.emplace_back("Invalid user.");
			break;
		}
	}

	if (invalid_user) {
		auto split = join_with_limit(errors, bot_max_embed_chars);

		error_message = dpp::message(command.channel_id, "");
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
								  .set_title("❌ Error while soft banning member(s): ")
								  .set_color(color::ERROR_COLOR)
								  .set_timestamp(time_now);
		if(split.size() == 1) {
			base_embed.set_description(split[0]);
			error_message.add_embed(base_embed);
		}
		else {
			for (auto const &error: errors) {
				auto embed{base_embed};
				embed.set_description(error);
				error_message.add_embed(embed);
			}
		}
		if (command.interaction) {
			error_message.set_flags(dpp::m_ephemeral); // Hidden error message
			if(split.size() == 1)
				(*command.interaction)->edit_response(error_message);
			else {
				message_paginator paginator{error_message, command};
				paginator.start();
			}
		}
		else { // It's an auto mod action
			auto transaction		 = pqxx::work{*command.connection};
			auto error_channel_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			if(error_channel_query.empty()) {
				error_message.set_content("This server hasn't set a channel for bot errors. So the errors are being "
										  "sent to your DMs:");
				command.bot->direct_message_create(command.author->user_id, error_message);
			}
			else {
				auto webhook_url = error_channel_query[0]["bot_error_logs"].as<std::string>();
				auto webhook = dpp::webhook{webhook_url};
				command.bot->execute_webhook(webhook, error_message);
			}
		}
		return;
	}
	check_permissions();
	if(cancel_operation)
		return;
	process_softbans();
	process_response();
}

void softban_wrapper::check_permissions() {
	if(members.empty() && users.empty()) {// No point in all this if there's no member/user in that vector
		cancel_operation = true;
		errors.emplace_back("❌ Error while parsing the list. It's possible that the provided ids were all invalid.");
		return;
	}

	auto const bot_member = dpp::find_guild_member(command.guild->id, command.bot->me.id);

	auto bot_roles = get_roles_sorted(bot_member);
	auto bot_top_role = bot_roles.front();

	auto author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	bool ignore_owner_repeat{false};

	pqxx::work transaction{*command.connection};
	auto protected_roles_query = transaction.exec_prepared("protected_roles", std::to_string(command.guild->id));
	transaction.commit();


	shared_vector<dpp::role> protected_roles;

	if(!protected_roles_query.empty()) {
		auto const protected_roles_field = protected_roles_query[0]["protected_roles"];
		internal::unique_vector<dpp::snowflake> protected_role_ids = parse_psql_array<dpp::snowflake>(protected_roles_field);
		reactaio::transform(protected_role_ids, protected_roles, [](dpp::snowflake const& role_id) {
			return std::make_shared<dpp::role>(*dpp::find_role(role_id));
		});
	}

	if(!bot_top_role->has_ban_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot doesn't have the appropriate permissions. Please make sure the Ban Members permission is enabled.");
	}

	for(auto const& member: members) {
		auto member_roles = get_roles_sorted(*member);
		auto member_top_role = *member_roles.begin();

		if(command.author->user_id == member->user_id) { // If for some reason you decided to soft ban yourself lol
			if(member->is_guild_owner()) {
				errors.emplace_back("❌ Why are you soft banning yourself, server owner? lmfao");
				ignore_owner_repeat = true;
			}
			else {
				errors.emplace_back("❌ You can't soft ban yourself lmao.");
			}
			cancel_operation = true;
		}

		if(!ignore_owner_repeat && member->is_guild_owner()) { // Banning the server owner lmfao
			errors.emplace_back("❌ You can't soft ban the server owner lmfao.");
			cancel_operation = true;
		}


		if(command.bot->me.id == member->user_id) { // If you decided to soft ban the bot (ReactAIO)
			errors.emplace_back("❌ Can't soft ban myself lmfao.");
			cancel_operation = true;
		}

		if(!protected_roles.empty()) {
			shared_vector<dpp::role> member_protected_roles;
			reactaio::set_intersection(protected_roles, member_roles, member_protected_roles);
			if(!member_protected_roles.empty()) { // If member has any of the protected roles.
				cancel_operation = true;
				std::vector<std::string> role_mentions;
				std::ranges::transform(member_protected_roles, std::back_inserter(role_mentions), [](const role_ptr& role){
					return role->get_mention();
				});
				std::string role_mentions_str = join(role_mentions, " , ");
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot soft ban.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to soft ban. Please "
										 "move the bot role above the members and below your staff roles.",
										 member->get_mention()));
			cancel_operation = true;
		}

		if(member_top_role->position > author_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than you do. You can't soft ban them.",
										 member->get_mention()));
			cancel_operation = true;
		}
	}

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while soft banning member(s): ")
								  .set_color(color::ERROR_COLOR)
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
				command.bot->direct_message_create(command.author->user_id, error_message);
			}
		}
	}
}

void softban_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, user_ptr const &user) {
	if (completion.is_error()) {
		auto error = completion.get_error();
		errors.push_back(std::format("❌ Unable to soft ban user **{}**. Error code {}: {}.", user->format_username(), error.code, error.human_readable));
		users_with_errors.insert(user);
	}
	else {
		auto transaction = pqxx::work{*command.connection};
		auto max_query = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
		auto max_id = std::get<0>(max_query.as<case_t>()) + 1;

		transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id,
								  reactaio::internal::mod_action_name::SOFT_BAN, std::to_string(command.author->user_id),
								  std::to_string(user->id), command.reason);
		transaction.commit();
	}
}

void softban_wrapper::process_softbans() {
	auto const* author_user = command.author->get_user();

	for (const auto& user: users) {
		if (command.interaction) { // If this is automod, DMing lots of users WILL result in a ratelimit

			std::string dm_message = std::format("You have been soft banned from {} by {}. Reason: {}.", command.guild->name,
									 author_user->format_username(), command.reason);
			command.bot->direct_message_create(user->id,dpp::message(dm_message));
		}
		command.bot->set_audit_reason(std::format("Softbanned by {} for reason: {}.", command.author->get_user()->format_username(), command.reason)).guild_ban_add(command.guild->id, user->id, max_ban_remove_seconds , [this, user](auto const& completion) {
			lambda_callback(completion, user);
		});

	}
}

void softban_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author->get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed		= dpp::embed()
								  .set_title("Error while soft banning member(s): ")
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
				(*command.interaction)->edit_response(message);
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
				command.bot->direct_message_create(command.author->user_id, automod_log);
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
		shared_vector<dpp::user> softbanned_users;
		std::vector<std::string> softbanned_usernames;
		std::vector<std::string> softbanned_mentions;

		reactaio::set_difference(users, users_with_errors, softbanned_users);

		std::ranges::transform(softbanned_users, std::back_inserter(softbanned_usernames), [](user_ptr const& user) {
			return std::format("**{}**", user->format_username());
		});

		std::ranges::transform(softbanned_users, std::back_inserter(softbanned_mentions), [](user_ptr const& user) {
			return user->get_mention();
		});

		auto usernames = join(softbanned_usernames, ", ");
		auto mentions  = join(softbanned_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if (softbanned_users.size() == 1) {
			title		= "Soft Banned";
			description = std::format("{} has been soft banned.", usernames);
			gif_url		= "https://media3.giphy.com/media/LOoaJ2lbqmduxOaZpS/giphy.gif";
		}
		else {
			title		= "Soft Banned";
			description = std::format("{} have been soft banned.", usernames);
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
		auto query = transaction.exec_prepared("ban_modlog", std::to_string(command.guild->id));
		transaction.commit();

		auto member_ban_webhook_url = query[0]["member_ban_add"];
		if(!member_ban_webhook_url.is_null()) {
			auto member_ban_webhook = dpp::webhook{member_ban_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(softbanned_users.size() > 1)
				embed_title = "Users soft banned: ";
			else {
				embed_title = "Users soft banned: ";
				embed_image_url = softbanned_users.at(0)->get_avatar_url();
			}

			description = std::format("{} have been soft banned.", usernames);
			time_now = std::time(nullptr);
			auto ban_log = dpp::embed()
								   .set_color(color::LOG_COLOR)
								   .set_title(embed_title)
								   .set_thumbnail(embed_image_url)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author->get_mention())
								   .add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(ban_log);

			command.bot->execute_webhook(member_ban_webhook, log);
		}
		auto modlog_webhook_url = query[0]["modlog"];
		if(!modlog_webhook_url.is_null()) {
			auto modlog_webhook = dpp::webhook{modlog_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(softbanned_users.size() > 1)
				embed_title = "Users banned: ";
			else {
				embed_title = "User banned: ";
				embed_image_url = softbanned_users.at(0)->get_avatar_url();
			}

			description = std::format("{} have been soft banned.", usernames);

			time_now = std::time(nullptr);
			auto ban_log = dpp::embed()
								   .set_color(color::LOG_COLOR)
								   .set_title(embed_title)
								   .set_thumbnail(embed_image_url)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author->get_mention())
								   .add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(ban_log);
			command.bot->execute_webhook(modlog_webhook, log);
		}
		auto public_modlog_webhook_url = query[0]["public_modlog"];
		if(!public_modlog_webhook_url.is_null()) {
			auto public_modlog_webhook = dpp::webhook{public_modlog_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(softbanned_users.size() > 1)
				embed_title = "Users soft banned: ";
			else {
				embed_title = "User soft banned: ";
				embed_image_url = softbanned_users.at(0)->get_avatar_url();
			}
			description = std::format("{} have been soft banned.", usernames);
			time_now = std::time(nullptr);
			auto ban_log = dpp::embed()
								   .set_color(color::LOG_COLOR)
								   .set_title(embed_title)
								   .set_thumbnail(embed_image_url)
								   .set_timestamp(time_now)
								   .set_description(description)
								   .add_field("Moderator: ", command.author->get_mention())
								   .add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(ban_log);
			command.bot->execute_webhook(public_modlog_webhook, log);
		}
	}
	if (command.interaction) {
		if(message.embeds.size() == 1)
			(*command.interaction)->edit_response(message);
		else {
			message_paginator paginator{message, command};
			paginator.start();
		}
		// Log command call
		pqxx::work transaction{*command.connection};
		transaction.exec_prepared("command_insert", std::to_string(command.guild->id), std::to_string(command.author->user_id),
								  reactaio::internal::mod_action_name::SOFT_BAN, dpp::utility::current_date_time());
		transaction.commit();
	}
	else
		command.bot->message_create(message);
}


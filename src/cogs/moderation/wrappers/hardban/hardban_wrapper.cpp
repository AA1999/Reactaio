//
// Created by arshia on 1/13/24.
//

#include "hardban_wrapper.h"

#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../mod_action.h"


void hardban_wrapper::wrapper_function() {
	for(auto const& member_or_user: snowflakes) {
		if (auto const member_pointer = std::get_if<member_ptr>(&member_or_user)) {
			members.insert(*member_pointer);
			users.insert((*member_pointer)->get_user());
		}
		else if(auto const* user_pointer = std::get_if<user_ptr>(&member_or_user)) {
			users.insert(*user_pointer);
		}
		else { // Will never happen but failsafe
			invalid_user = true;
			errors.emplace_back("Invalid user.");
			break;
		}
	}

	if (invalid_user) {
		auto const split = join_with_limit(errors, bot_max_embed_chars);

		error_message = dpp::message(command.channel_id, "");
		auto const time_now = std::time(nullptr);
		auto base_embed	= dpp::embed()
							.set_title("❌ Error while hard banning member(s): ")
							.set_color(ERROR_COLOR)
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
				auto const webhook_url = error_channel_query[0]["bot_error_logs"].as<std::string>();
				auto const webhook = dpp::webhook{webhook_url};
				command.bot->execute_webhook(webhook, error_message);
			}
		}
		return;
	}
	check_permissions();
	if(cancel_operation)
		return;
	process_hardbans();
	process_response();
}

void hardban_wrapper::check_permissions() {

	if(members.empty() && users.empty()) { // No point if no member/user are there.
		cancel_operation = true;
		errors.emplace_back("❌ Error while parsing the list. It's possible that the provided ids were all invalid.");
		return;
	}

	if(!command.author->is_guild_owner()) {
		cancel_operation = true;
		errors.emplace_back("❌ Can't hard ban anyone unless you're the server owner. This is a hardcoded condition and cannot be changed.");
		return; // If you're not the server owner, the other errors don't matter.
	}

	auto const bot_user = std::make_unique<dpp::user>(command.bot->me);
	auto const bot_member = find_guild_member(command.guild->id, bot_user->id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();

	pqxx::work transaction{*command.connection};
	auto protected_roles_query = transaction.exec_prepared("protected_roles", std::to_string(command.guild->id));
	transaction.commit();


	shared_vector<dpp::role> protected_roles;

	if(!protected_roles_query.empty()) {
		auto const protected_roles_field = protected_roles_query[0]["protected_roles"];
		internal::unique_vector<dpp::snowflake> protected_role_snowflakes = parse_psql_array<dpp::snowflake>(protected_roles_field);
		reactaio::transform(protected_role_snowflakes, protected_roles, [](const dpp::snowflake role_id) {
			return std::make_shared<dpp::role>(*find_role(role_id));
		});
	}

	if(!bot_top_role->has_ban_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot doesn't have the appropriate permissions. Please make sure the Ban Members permission is enabled.");
	}

	for(auto const& member: members) {
		auto const member_roles = get_roles_sorted(*member);
		auto const& member_top_role = member_roles.front();

		if(member->is_guild_owner()) { // If for some reason you felt like hardbanning yourself, being the server owner.
			cancel_operation = true;
			errors.emplace_back("❌ Why are you trying to hard ban yourself, server owner? lmfao");
		}
		if(command.bot->me.id == member->user_id) { // If you decided to ban the bot (ReactAIO)
			errors.emplace_back("❌ Can't hard ban myself lmfao.");
			cancel_operation = true;
		}

		if(!protected_roles.empty()) {
			shared_vector<dpp::role> member_protected_roles;
			reactaio::set_intersection(protected_roles, member_roles, member_protected_roles);
			if(!member_protected_roles.empty()) { // If member has any of the protected roles.
				cancel_operation = true;
				std::vector<std::string> role_mentions;
				std::ranges::transform(member_protected_roles, std::back_inserter(role_mentions), [](role_ptr const& role){
					return role->get_mention();
				});
				std::string role_mentions_str = join(role_mentions, " , ");
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot hard ban.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to hard ban. Please "
										 "move the bot role above the members and below your staff roles.", member->get_mention()));
			cancel_operation = true;
		}
	}
	if(cancel_operation) {
		auto const organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while hard banning member(s): ")
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
		else {
			auto webhook_url_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			transaction.commit();
			auto const bot_error_webhook_url_field = webhook_url_query[0]["bot_error_logs"];
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

void hardban_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, user_ptr const &user) {
	if (completion.is_error()) {
		auto error = completion.get_error();
		errors.push_back(std::format("❌ Unable to hard ban user **{}**. Error code {}: {}.", user->format_username(), error.code, error.human_readable));
		users_with_errors.insert(user);
	}
	else {
		auto transaction = pqxx::work{*command.connection};
		auto const max_query = transaction.exec_prepared1("casecount", std::to_string(command.guild->id));
		auto max_id = std::get<0>(max_query.as<case_t>()) + 1;
		transaction.exec_prepared("modcase_insert", std::to_string(command.guild->id), max_id, internal::mod_action_name::HARD_BAN,
								  std::to_string(command.author->user_id), std::to_string(user->id), command.reason);
		transaction.commit();
	}
}

void hardban_wrapper::process_hardbans() {
	uint ban_remove_days{0};
	constexpr uint TO_SECONDS = 24 * 3600;

	pqxx::work transaction{*command.connection};
	auto const ban_remove_days_query = transaction.exec_prepared("get_ban_remove_days", std::to_string(command.guild->id));
	transaction.commit();
	if(!ban_remove_days_query.empty()) {
		ban_remove_days = ban_remove_days_query[0]["ban_remove_days"].as<uint>();
		ban_remove_days *= TO_SECONDS;
	}

	auto* author_user = command.author->get_user();

	for (auto const& user : users) {
		if (command.interaction) { // If this was done in automod, DMing lots of users WILL result in a rate limit

			std::string dm_message;

			dm_message = std::format("You have been hard banned from {} by {}. Reason: {}. Only they can unban you.", command.guild->name,
										 author_user->format_username(), command.reason);
			command.bot->direct_message_create(user->id,dpp::message(dm_message));
		}
		command.bot->set_audit_reason(std::format("Hardbanned by {} for reason: {}", command.author->get_user()->format_username(), command.reason)).guild_ban_add(command.guild->id, user->id, ban_remove_days , [this, user](auto const& completion) {
			lambda_callback(completion, user);
		});
	}
}

void hardban_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author->get_user();

	if (has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed	= dpp::embed()
							.set_title("Error while hard banning member(s): ")
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
		shared_vector<dpp::user> hard_banned_users;
		std::vector<std::string> hard_banned_usernames;
		std::vector<std::string> hard_banned_mentions;

		reactaio::set_difference(users, users_with_errors, hard_banned_users);

		std::ranges::transform(hard_banned_users, std::back_inserter(hard_banned_usernames), [](user_ptr const& user) {
			return std::format("**{}**", user->format_username());
		});

		std::ranges::transform(hard_banned_users, std::back_inserter(hard_banned_mentions), [](user_ptr const& user) {
			return user->get_mention();
		});

		auto usernames = join(hard_banned_usernames, ", ");
		auto mentions  = join(hard_banned_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if (hard_banned_users.size() == 1) {
			title		= "Hard banned";
			description = std::format("{} has been hard banned", usernames);
			gif_url		= "https://media3.giphy.com/media/LOoaJ2lbqmduxOaZpS/giphy.gif";
		}
		else {
			title		= "Hard banned";
			description = std::format("{} have been hard banned", usernames);
			gif_url		= "https://i.gifer.com/1Daz.gif";
		}

		if(duration) {
			auto time_now = std::chrono::system_clock::now();
			auto time_delta = duration->to_seconds();
			auto future = time_now + time_delta;
			std::string time_future_relative = timestamp(future.time_since_epoch().count(),
																	   dpp::utility::time_format::tf_relative_time);
			description.append(std::format(" until {}.", time_future_relative));
		}
		else
			description.append(".");

		auto time_now = std::time(nullptr);

		auto response = dpp::embed()
								.set_color(RESPONSE_COLOR)
								.set_title(title)
								.set_description(description)
								.set_image(gif_url)
								.set_timestamp(time_now);
		response.add_field("Moderator: ", author_user->get_mention());
		response.add_field("Reason: ", command.reason);

		message.add_embed(response);

		// Modlogs

		auto transaction = pqxx::work{*command.connection};
		auto query = transaction.exec_prepared("ban_modlog", std::to_string(command.guild->id));
		transaction.commit();

		auto const member_ban_webhook_url = query[0]["member_ban_add"];
		if(!member_ban_webhook_url.is_null()) {
			auto member_ban_webhook = dpp::webhook{member_ban_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(hard_banned_users.size() > 1)
				embed_title = "Users hard banned: ";
			else {
				embed_title = "Users hard banned: ";
				embed_image_url = hard_banned_users.at(0)->get_avatar_url();
			}

			description = std::format("{} have been hard banned.", usernames);

			time_now = std::time(nullptr);
			auto ban_log = dpp::embed()
								   .set_color(LOG_COLOR)
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
			if(hard_banned_users.size() > 1)
				embed_title = "Users hard banned: ";
			else {
				embed_title = "User hard banned: ";
				embed_image_url = hard_banned_users.at(0)->get_avatar_url();
			}

			description = std::format("{} have been hard banned.", usernames);

			time_now = std::time(nullptr);
			auto ban_log = dpp::embed()
								   .set_color(LOG_COLOR)
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
			if(hard_banned_users.size() > 1)
				embed_title = "Users hard banned: ";
			else {
				embed_title = "User hard banned: ";
				embed_image_url = hard_banned_users.at(0)->get_avatar_url();
			}
			description = std::format("{} have been hard banned.", usernames);
			time_now = std::time(nullptr);
			auto const ban_log = dpp::embed()
								   .set_color(LOG_COLOR)
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
								  internal::mod_action_name::HARD_BAN, dpp::utility::current_date_time());
		transaction.commit();
	}
	else
		command.bot->message_create(message);
}

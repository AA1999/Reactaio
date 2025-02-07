//
// Created by arshia on 2/24/23.
//

#include "ban_wrapper.h"
#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../mod_action.h"

#include <algorithm>
#include <chrono>
#include <dpp/dpp.h>
#include <format>
#include <memory>


void ban_wrapper::wrapper_function() {
	for(auto& member_or_user: snowflakes) {
		if(auto const member_pointer = std::get_if<member_ptr>(&member_or_user)) {
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

	if(invalid_user) {
		auto const split = join_with_limit(errors, bot_max_embed_chars);
		error_message = dpp::message(command.channel_id, "");
		auto const time_now = std::time(nullptr);
		auto base_embed	= dpp::embed()
				.set_title("❌ Error while banning member(s): ")
				.set_color(ERROR_COLOR)
				.set_timestamp(time_now);
		if(split.size() == 1) {
			base_embed.set_description(split[0]);
			error_message.add_embed(base_embed);
		}
		else {
			for(auto const &error: errors) {
				auto embed{base_embed};
				embed.set_description(error);
				error_message.add_embed(embed);
			}
		}
		if(command.interaction) {
			error_message.set_flags(dpp::m_ephemeral); // Hidden error message
			if(split.size() == 1)
				(*command.interaction)->edit_response(error_message);
			else {
				message_paginator paginator{error_message, command};
				paginator.start();
			}
		}
		else { // It's an auto mod action
			auto transaction = pqxx::work{*command.connection};
			auto error_channel_query = transaction.exec_prepared("botlog", std::to_string(command.guild->id));
			if(error_channel_query.empty()) {
				error_message.set_content("This server hasn't set a channel forbot errors. So the errors are being "
				                          "sent to your DMs:");
				command.bot->direct_message_create(command.author->user_id, error_message);
			}
			else {
				const auto webhook_url = error_channel_query[0]["bot_error_logs"].as<std::string>();
				const auto webhook = dpp::webhook{webhook_url};
				command.bot->execute_webhook(webhook, error_message);
			}
		}
		return;
	}

	check_permissions();
	if(cancel_operation)
		return;

	process_bans();
	process_response();
}

void ban_wrapper::lambda_callback(const dpp::confirmation_callback_t &completion, user_ptr const &user) {
	if(completion.is_error()) {
		auto error = completion.get_error();
		errors.push_back(std::format("❌ Unable to ban user **{}**. Error code {}: {}.", user->format_username(), error.code, error.human_readable));
		users_with_errors.insert(user);
	}
	else
		log_modcase(internal::mod_action_name::BAN);
}

void ban_wrapper::process_bans() {

	uint ban_remove_days{0};
	constexpr uint TO_SECONDS = 24 * 3600;

	if(command.delete_message_days == 0) {
		pqxx::work transaction{*command.connection};
		auto ban_remove_days_query = transaction.exec_prepared("get_ban_remove_days", std::to_string(command.guild->id));
		transaction.commit();
		if(!ban_remove_days_query.empty()) {
			ban_remove_days = ban_remove_days_query[0]["ban_remove_days"].as<uint>();
			ban_remove_days *= TO_SECONDS;
		}
	}
	else
		ban_remove_days = command.delete_message_days;

	pqxx::work transaction{*command.connection};

	id_t ban_id = 1;
	auto ban_id_row = transaction.exec_prepared1("get_ban_id", std::to_string(command.guild->id));
	transaction.commit();
	if(!ban_id_row["ban_id"].is_null()) {
		ban_id = ban_id_row["ban_id"].as<id_t>() + 1;
	}

	auto* author_user = command.author->get_user();

	for(auto const& user : users) {
		if(command.interaction) { // ifthis is automod, DMing lots of users WILL result in a ratelimit

			std::string dm_message;

			if(duration) {
				auto time_now = std::chrono::system_clock::now();
				auto time_delta = duration->to_seconds();
				auto future = time_now + time_delta;
				std::string time_future_relative = timestamp(future.time_since_epoch().count(),
																		   dpp::utility::time_format::tf_relative_time);
				dm_message = std::format("You have been banned by from {} by {} until {}. Reason: {}", command.guild->name,
				                         author_user->format_username(), time_future_relative, command.reason);
				std::string const now_str = std::format(iso_format, time_now);
				std::string const end_str = std::format(iso_format, future);

				transaction.exec_prepared0("tempban", std::to_string(ban_id), std::to_string(user->id), std::to_string(command.guild->id), std::to_string(command.author->user_id), now_str, end_str, command.reason);
				transaction.commit();
			}
			else
				dm_message = std::format("You have been banned from {} by {}. Reason: {}.", command.guild->name,
				                         author_user->format_username(), command.reason);
			command.bot->direct_message_create(user->id,dpp::message(dm_message));
		}


		command.bot->set_audit_reason(std::format("Banned by {} with reason: {}", command.author->get_user()->format_username(), command.reason)).guild_ban_add(command.guild->id, user->id, ban_remove_days , [this, user](auto const& completion) {
			lambda_callback(completion, user);
		});
	}
}

void ban_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const *author_user = command.author->get_user();

	if(has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
								  .set_title("Error while banning member(s): ")
								  .set_color(ERROR_COLOR)
								  .set_timestamp(time_now);
		if(format_split.size() == 1) {
			base_embed.set_description(format_split[0]);
			message.add_embed(base_embed);
		}
		else {
			for(auto const &error: format_split) {
				auto embed{base_embed};
				embed.set_description(error);
				message.add_embed(embed);
			}
		}
		if(command.interaction) {
			if(are_all_errors())
				message.set_flags(dpp::m_ephemeral);// Invisible error message.
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
		shared_vector<dpp::user> banned_users;
		std::vector<std::string> banned_usernames;
		std::vector<std::string> banned_mentions;

		reactaio::set_difference(users, users_with_errors, banned_users);

		std::ranges::transform(banned_users, std::back_inserter(banned_usernames), [](user_ptr const &user) {
			return std::format("**{}**", user->format_username());
		});

		std::ranges::transform(banned_users, std::back_inserter(banned_mentions), [](user_ptr const &user) {
			return user->get_mention();
		});

		auto usernames = join(banned_usernames, ", ");
		auto mentions = join(banned_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if(banned_users.size() == 1) {
			title = "Banned";
			description = std::format("{} has been banned", usernames);
			gif_url = "https://media3.giphy.com/media/LOoaJ2lbqmduxOaZpS/giphy.gif";
		}
		else {
			title = "Banned";
			description = std::format("{} have been banned", usernames);
			gif_url = "https://i.gifer.com/1Daz.gif";
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

		auto member_ban_webhook_url = query[0]["member_ban_add"];
		if(!member_ban_webhook_url.is_null()) {
			auto member_ban_webhook = dpp::webhook{member_ban_webhook_url.as<std::string>()};
			std::string embed_title, embed_image_url;
			if(banned_users.size() > 1)
				embed_title = "Users banned: ";
			else {
				embed_title = "Users banned: ";
				embed_image_url = banned_users.at(0)->get_avatar_url();
			}

			description = std::format("{} have been banned", usernames);

			if(duration) {
				auto now = std::chrono::system_clock::now();
				auto time_delta = duration->to_seconds();
				auto future = now + time_delta;
				std::string time_future_relative = timestamp(future.time_since_epoch().count(),
															 dpp::utility::time_format::tf_relative_time);
				description.append(std::format(" until {}.", time_future_relative));
			}
			else
				description.append(".");
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
			if(banned_users.size() > 1)
				embed_title = "Users banned: ";
			else {
				embed_title = "User banned: ";
				embed_image_url = banned_users.at(0)->get_avatar_url();
			}

			description = std::format("{} have been banned", usernames);
			if(duration) {
				auto now = std::chrono::system_clock::now();
				auto time_delta = duration->to_seconds();
				auto future = now + time_delta;
				std::string time_future_relative = timestamp(future.time_since_epoch().count(),
															 dpp::utility::time_format::tf_relative_time);
				description.append(std::format(" until {}.", time_future_relative));
			} 
			else
				description.append(".");

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
			if(banned_users.size() > 1)
				embed_title = "Users banned: ";
			else {
				embed_title = "User banned: ";
				embed_image_url = banned_users.at(0)->get_avatar_url();
			}
			description = std::format("{} have been banned", usernames);
			if(duration) {
				auto now = std::chrono::system_clock::now();
				auto time_delta = duration->to_seconds();
				auto future = now + time_delta;
				std::string time_future_relative = timestamp(future.time_since_epoch().count(),
															 dpp::utility::time_format::tf_relative_time);
				description.append(std::format(" until {}.", time_future_relative));
			}
			else
				description.append(".");
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
		log_command_invoke(internal::mod_action_name::BAN);
	}
	else
        command.bot->message_create(message);
}

void ban_wrapper::check_ban_possible(shared_vector<dpp::role> const& protected_roles) {
	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();
	auto const author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	bool is_owner{false};

	for(auto const &member: members) {
		auto const member_roles = get_roles_sorted(*member);
		auto const& member_top_role = member_roles.front();

		if(command.author->user_id == member->user_id) {// ifforsome reason you decided to ban yourself lol
			if(member->is_guild_owner()) {
				errors.emplace_back("❌ Why are you banning yourself, server owner? lmfao");
				is_owner = true;
			}
			else {
					errors.emplace_back("❌ You can't ban yourself lmao.");
			}
			cancel_operation = true;
		}
		if(!is_owner && member->is_guild_owner()) {
			errors.emplace_back("❌ You can't ban the server owner lmfao.");
			cancel_operation = true;
		}


		if(command.bot->me.id == member->user_id) {// ifyou decided to ban the bot (ReactAIO)
			errors.emplace_back("❌ Can't ban myself lmfao.");
			cancel_operation = true;
		}

		if(!protected_roles.empty()) {

			shared_vector<dpp::role> member_protected_roles;
			reactaio::set_intersection(protected_roles, member_roles, member_protected_roles);
			if(!member_protected_roles.empty()) {// IfR member has any of the protected roles.
				cancel_operation = true;
				std::vector<std::string> role_mentions;
				std::ranges::transform(member_protected_roles, std::back_inserter(role_mentions), [](const role_ptr &role) {
					return role->get_mention();
				});
				std::string role_mentions_str = join(role_mentions, " , ");
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot ban.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to ban. "
													"Please move the bot role above the members and below your staff roles.",
											member->get_mention()));
			cancel_operation = true;
		}

		if(member_top_role->position > author_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than you do. You can't ban them.",
										 member->get_mention()));
			cancel_operation = true;
		}
	}
}

void ban_wrapper::check_permissions() {

	if(members.empty() && users.empty()) {// No point in all this ifthere's no member/user in that vector
		cancel_operation = true;
		errors.emplace_back("❌ Error while parsing the list. It's possible that the provided ids were all invalid.");
		return;
	}

	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();
	auto const author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	pqxx::work transaction{*command.connection};
	auto const protected_roles = get_protected_roles();

	if(!bot_top_role->has_ban_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot doesn't have the appropriate permissions. Please make sure the Ban Members permission is enabled.");
	}
	
	auto const roles = get_permitted_roles(internal::mod_action_name::BAN);
	bool command_permitted{true};

	if (roles.empty() && !author_top_role->has_ban_members() || (!roles.empty() && !author_top_role->has_ban_members() && !roles.contains(author_top_role))) {
		cancel_operation = true;
		command_permitted = false;
		errors.emplace_back("❌ You do not have permission to run this command.");
	}
	if(command_permitted)
		check_ban_possible(protected_roles);

	if(cancel_operation) {
		auto const organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
		                          .set_title("Error while banning member(s): ")
		                          .set_color(ERROR_COLOR)
		                          .set_timestamp(time_now);
		if(organized_errors.size() == 1) {
			base_embed.set_description(organized_errors[0]);
			error_message.add_embed(base_embed);
		}
		else {
			for(auto const &error: organized_errors) {
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

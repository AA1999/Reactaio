//
// Created by arshia on 3/4/23.
//

#include "mute_wrapper.h"
#include "../../../core/algorithm.h"
#include "../../../core/colors.h"
#include "../../../core/consts.h"
#include "../../../core/discord/message_paginator.h"
#include "../../../core/helpers.h"
#include "../../mod_action.h"

#include <memory>


void mute_wrapper::wrapper_function() {
	check_permissions();

	if(cancel_operation)
		return;

	process_mutes();
	process_response();
}

void mute_wrapper::check_mute_possible(shared_vector<dpp::role> const& protected_roles) {
	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);
	auto const bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();
	auto const author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	bool is_owner{false};
	for(auto const& member: members) {
		auto const member_roles = get_roles_sorted(*member);
		auto const member_top_role = *member_roles.begin();

		if(command.author->user_id == member->user_id) {// ifforsome reason you decided to mute yourself lol
			if(member->is_guild_owner()) {
				errors.emplace_back("❌ Why are you muting yourself, server owner? lmfao");
				is_owner = true;
			} else {
				errors.emplace_back("❌ You can't mute yourself lmao.");
			}
			cancel_operation = true;
		}

		if(!is_owner && member->is_guild_owner()) {
			errors.emplace_back("❌ You can't mute the server owner lmfao.");
			cancel_operation = true;
		}


		if(command.bot->me.id == member->user_id) {// ifyou decided to mute the bot (ReactAIO)
			errors.emplace_back("❌ Can't mute myself lmfao.");
			cancel_operation = true;
		}

		if(!protected_roles.empty()) {

			shared_vector<dpp::role> member_protected_roles;
			reactaio::set_intersection(protected_roles, member_roles, member_protected_roles);
			if(!member_protected_roles.empty()) {// ifmember has any of the protected roles.
				cancel_operation = true;
				std::vector<std::string> role_mentions;
				std::ranges::transform(member_protected_roles, std::back_inserter(role_mentions), [](const role_ptr &role) {
					return role->get_mention();
				});
				std::string role_mentions_str = join(role_mentions, " , ");
				errors.push_back(std::format("❌ Member has the protected roles: {}. Cannot mute.", role_mentions_str));
			}
		}

		if(member_top_role->position > bot_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than the bot. Unable to mute. Please "
										 "move the bot role above the members and below your staff roles.",
										 member->get_mention()));
			cancel_operation = true;
		}

		if(member_top_role->position > author_top_role->position) {
			errors.push_back(std::format("❌ {} has a higher role than you do. You can't mute them.",
										 member->get_mention()));
			cancel_operation = true;
		}
	}
}

void mute_wrapper::check_permissions() {
	auto const bot_member = find_guild_member(command.guild->id, command.bot->me.id);

	auto bot_roles = get_roles_sorted(bot_member);
	auto const& bot_top_role = bot_roles.front();

	auto author_roles = get_roles_sorted(*command.author);
	auto const& author_top_role = author_roles.front();

	pqxx::work transaction{*command.connection};

	if(!bot_top_role->has_moderate_members()) {
		cancel_operation = true;
		errors.emplace_back("❌ Bot lacks the appropriate permissions. Please check ifthe bot has Moderate Members permission.");
	}

	bool command_permitted{true};

	auto const protected_roles = get_protected_roles();
	auto const roles = get_permitted_roles(internal::mod_action_name::MUTE);
	if(roles.empty() && !author_top_role->has_kick_members() || (!roles.empty() && !author_top_role->has_kick_members() && !roles.contains(author_top_role))) {
		cancel_operation = true;
		command_permitted = false;
		errors.emplace_back("❌ You do not have permission to run this command.");
	}
	if(command_permitted)
		check_mute_possible(protected_roles);

	if(cancel_operation) {
		auto organized_errors = join_with_limit(errors, bot_max_embed_chars);
		auto time_now = std::time(nullptr);
		auto base_embed = dpp::embed()
		                          .set_title("Error while muting member(s): ")
		                          .set_color(ERROR_COLOR)
		                          .set_timestamp(time_now);
		for(auto const& error: organized_errors) {
			auto embed{base_embed};
			embed.set_description(error);
			error_message.add_embed(embed);
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

void mute_wrapper::lambda_callback(const dpp::confirmation_callback_t& completion, member_ptr const& member) {
	if(completion.is_error()) {
		auto const error = completion.get_error();
		members_with_errors.insert(member);
		errors.push_back(std::format("❌ Error code {}: {}", error.code, error.message));
		return;
	}
	pqxx::work transaction{*command.connection};

	auto const now = std::chrono::system_clock::now();
	std::chrono::seconds time_delta;
	if(duration)
		time_delta = duration->to_seconds();
	else if(use_timeout)
		time_delta = std::chrono::seconds{max_timeout_seconds};
	auto const future = now + time_delta;
	auto const future_relative = timestamp(future.time_since_epoch().count(), dpp::utility::time_format::tf_relative_time);

	std::string const dm_message = std::format("You have been {} in {} by {} until {}.", use_timeout ? "Timed out" : "Muted", command.guild->name, command.author->get_user()->format_username(), duration ? future_relative : "unmuted");
	if(!duration && use_timeout) {
		std::uint64_t id{1};

		auto const timeout_id_row = transaction.exec_prepared1("get_timeout_id", std::to_string(command.guild->id));
		if(!timeout_id_row["timeout_id"].is_null())
			id = timeout_id_row["timeout_id"].as<std::uint64_t>();
		transaction.exec_prepared("permanent_timeout", id, std::to_string(member->user_id),
									std::to_string(command.guild->id), std::to_string(command.author->user_id), command.reason);
		transaction.commit();
	}
	else if(duration) {
		std::uint64_t id{1};
		auto const mute_id_row = transaction.exec_prepared1("get_mute_id", std::to_string(command.guild->id));
		if(!mute_id_row["mute_id"].is_null())
			id = mute_id_row["mute_id"].as<std::uint64_t>();
		transaction.exec_prepared("tempmute", id, std::to_string(member->user_id), std::to_string(command.guild->id), now.time_since_epoch().count(), future.time_since_epoch().count());
		transaction.commit();

	}
	command.bot->direct_message_create(member->user_id, dpp::message{dm_message});
}

void mute_wrapper::process_mutes() {
	pqxx::work transaction{*command.connection};

	auto const user_timeout_row = transaction.exec_prepared1("check_timeout", std::to_string(command.guild->id));
	use_timeout = user_timeout_row["use_timeout"].is_null() ? true : user_timeout_row["use_timeout"].as<bool>();

	duration = parse_human_time(command.duration);
	if(use_timeout) {
		if(duration) {
			if(duration->seconds() > max_timeout_seconds) {
				errors.emplace_back("❌ Invalid timeout duration. Timeouts can only be max of 28 days.");
			}
			else {
				auto const now = std::chrono::system_clock::now();
				auto const time_delta = duration->to_seconds();
				auto const future = now + time_delta;
				auto const future_ms = future.time_since_epoch().count();
				for(auto const& member: members) {
					command.bot->set_audit_reason(std::format("Timed out by {} forreason: {}", command.author->get_user()->format_username(), command.reason)).guild_member_timeout(command.guild->id, member->user_id, future_ms, [this, member](auto& completion){
						lambda_callback(completion, member);
					});
				}
			}
		}
		else {
			for(auto const& member: members) {
				command.bot->set_audit_reason(std::format("Timed out by {} forreason: {}", command.author->get_user()->format_username(), command.reason)).guild_member_timeout(command.guild->id, member->user_id, max_timeout_seconds, [this, member](auto& completion) {
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
			command.bot->set_audit_reason(std::format("Muted by {} for{} forreason: {}", command.author->get_user()->format_username(), duration->to_string(true), command.reason)).guild_member_add_role(command.guild->id, member->user_id, mute_role_id, [this, member](auto& completion){
				lambda_callback(completion, member);
			});
		}
	}
}

void mute_wrapper::process_response() {
	auto message = dpp::message(command.channel_id, "");
	auto const* author_user = command.author->get_user();

	if(has_error()) {
		auto format_split = join_with_limit(errors, bot_max_embed_chars);
		auto const time_now = std::time(nullptr);
		auto base_embed	= dpp::embed()
							  .set_title("Error while muting member(s): ")
							  .set_color(ERROR_COLOR)
							  .set_timestamp(time_now);
		if(command.interaction) {
			if(format_split.size() == 1) {
				base_embed.set_description(format_split[0]);
				message.add_embed(base_embed);
			}
			else {
				for(auto const& error: format_split) {
					auto embed{base_embed};
					embed.set_description(error);
					message.add_embed(embed);
				}
			}
			if(are_all_errors())
				message.set_flags(dpp::m_ephemeral);// If there's no successful mute, no reason to show the errors publicly.
		}
		else
			invoke_error_webhook();
	}
	if(!are_all_errors()) {
		shared_vector<dpp::guild_member> muted_members;
		std::vector<std::string> muted_usernames;
		std::vector<std::string> muted_mentions;

		reactaio::set_difference(members, members_with_errors, muted_members);

		std::ranges::transform(muted_members, std::back_inserter(muted_usernames), [](member_ptr const& member) {
			return std::format("**{}**", member->get_user()->format_username());
		});

		std::ranges::transform(muted_members, std::back_inserter(muted_mentions), [](member_ptr const& member) {
			return member->get_mention();
		});

		auto usernames = join(muted_usernames, ", ");
		auto mentions  = join(muted_mentions, ", ");

		std::string title;
		std::string description;
		std::string gif_url;

		if(muted_members.size() == 1) {
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
				embed_image_url = muted_members.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto mute_log = dpp::embed()
									.set_color(LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been muted.", usernames))
									.add_field("Moderator: ", command.author->get_mention())
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
				embed_image_url = muted_members.at(0)->get_avatar_url();
			}
			time_now = std::time(nullptr);
			auto mute_log = dpp::embed()
									.set_color(LOG_COLOR)
									.set_title(embed_title)
									.set_thumbnail(embed_image_url)
									.set_timestamp(time_now)
									.set_description(std::format("{} have been muted.", usernames))
									.add_field("Moderator: ", command.author->get_mention())
									.add_field("Reason: ", std::string{command.reason});
			dpp::message log{command.channel_id, ""};
			log.add_embed(mute_log);
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
		log_command_invoke(use_timeout ? internal::mod_action_name::TIMEOUT : internal::mod_action_name::MUTE);
	}
	else
		command.bot->message_create(message);
}

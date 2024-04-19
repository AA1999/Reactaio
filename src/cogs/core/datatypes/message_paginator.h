//
// Created by arshia on 3/1/23.
//

#pragma once

#include "../aliases.h"
#include "../discord_command.h"

#include <dpp/dpp.h>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <format>

/**
 * @brief message_paginator - A utility class to show a serires of texts/embes in one message. Buttons are used for navigation.
 */
class message_paginator {
	discord_command m_command;
	dpp::message m_message;
	std::vector<dpp::embed> pages;
	std::array<dpp::component, 4> buttons{};
	dpp::component action_row;
	ushort current_page{0};
	bool const use_embeds;
	std::vector<std::string> messages;

	enum action: std::uint8_t {
		SKIP_FIRST = 0,
		BACKWARD = 1,
		FORWARD = 2,
		SKIP_LAST = 3
	};

	constexpr static std::string_view SKIP_FIRST_FORMAT = "{}_{}_{}_skip_first";
	constexpr static std::string_view SKIP_LAST_FORMAT = "{}_{}_{}_skip_last";
	constexpr static std::string_view FORWARD_FORMAT = "{}_{}_{}_forward";
	constexpr static std::string_view BACKWARD_FORMAT = "{}_{}_{}_backward";

	inline static ullong id{0}; // Unique id for each paginator

	std::string const forward_id;
	std::string const backward_id;
	std::string const skip_last_id;
	std::string const skip_first_id;

	/**
	 * @brief forward - Goes to the next page. Goes back to page 0 if at the last page already.
	 */
	void forward();

	/**
	 * @brief backward - Goes to the previous page. Goes back to the last page if the
	 */
	void backward();

	/**
	 * @brief skip_last - Jumps to the last page.
	 */
	void skip_last();

	/**
	 * @brief skip_first - Jumps to the first page.
	 */
	void skip_first();


public:
	~message_paginator();
	message_paginator() = delete;

	/**
	 * @brief One of the two constructors used to create the message paginator using an existing message with embeds.
	 * @param message The message to get the embeds from.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	message_paginator(dpp::message message, discord_command& command): m_command(std::move(command)),
																			  m_message(std::move(message)),
																			  pages(m_message.embeds),
																			  use_embeds{true},
																			  forward_id(std::format(FORWARD_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id)),
															                  backward_id(std::format(BACKWARD_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id)),
																			  skip_last_id(std::format(SKIP_LAST_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id)),
															                  skip_first_id(std::format(SKIP_FIRST_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id))
	{
		id++;
	}

	/**
	 * @brief Constructor for creating a paginator from a list of message contents with an existing message to be edited later.
	 * @param message The message to edit with the paginator in it.
	 * @param messages The series of messages to put inside the paginator.
	 * @param command This is a command moderation_command object that includes every detail about the command that was invoked (whether it was a slash command or an automod response)
	 */
	message_paginator(dpp::message message, const std::vector<std::string>& messages , discord_command & command): m_command(command), m_message(std::move(message)),
					use_embeds{false},
					messages(messages),
	                forward_id(std::format(FORWARD_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id)),
	                backward_id(std::format(BACKWARD_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id)),
					skip_last_id(std::format(SKIP_LAST_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id)),
	                skip_first_id(std::format(SKIP_FIRST_FORMAT, m_command.author->user_id.str(), m_command.channel_id.str(), id))

	{
		id++;
	}

	/**
	 * @brief add_page - Adds a page to the paginator.
	 * @param embed Embed to add to the paginator.
	 * @throw std::invalid_argument if the paginator already has message objects and not embeds.
	 */
	void add_page(const dpp::embed &embed);

	/**
	 * @brief add_page - Adds a page to the paginator.
	 * @param page Page to be added to the paginator.
	 * @throw std::invalid_argument if the paginator already has embeds and not message objects.
	 */
	void add_page(const std::string& page);

	/**
	 * @brief remove_page - Removes the specific page number from the paginator.
	 * @param page The page number to remove
	 * @throw std::out_of_range if the page number is above the paginator size.
	 */
	void remove_page(std::size_t page);

	/**
	 * @brief size - Returns the length of the paginator.
	 * @return  The length of the paginator.
	 */
	[[nodiscard]] constexpr std::size_t size() const {
		return is_embed_paginator() ? pages.size() : messages.size();
	}

	/**
	 * @brief start - Starts the paginator and activates the buttons.
	 */
	void start();

	[[nodiscard]] constexpr bool is_embed_paginator() const {
		return use_embeds;
	}

};

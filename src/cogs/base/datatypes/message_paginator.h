//
// Created by arshia on 3/1/23.
//

#pragma once

#include "../aliases.h"
#include "../base_command.h"

#include <dpp/dpp.h>
#include <string>
#include <vector>
#include <array>
#include <format>

/**
 * @brief message_paginator - A utility class to show a serires of texts/embes in one message. Buttons are used for navigation.
 */
class message_paginator {
	std::vector<dpp::embed> pages;
	std::array<dpp::component, 4> buttons{};
	dpp::component action_row;
	dpp::message message;
	base_command command;
	ushort current_page{0};
	bool const use_embeds;
	std::vector<std::string> messages;

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

	const static ushort SKIP_FIRST{0};
	const static ushort BACKWARD{1};
	const static ushort FORWARD{2};
	const static ushort SKIP_LAST{3};

	inline static ullong id{0}; // Unique id for each paginator

	std::string const forward_id;
	std::string const backward_id;
	std::string const skip_last_id;
	std::string const skip_first_id;

public:
	~message_paginator();
	message_paginator() = delete;
	message_paginator(const dpp::message& message, base_command& command): pages(message.embeds), message(message),
	                  command(command), action_row(), use_embeds{true},
	                  forward_id(std::format("{}_{}_{}_forward", command.author.user_id.str(), command.channel_id.str(), id)),
	                  backward_id(std::format("{}_{}_{}_backward", command.author.user_id.str(), command.channel_id.str(), id)),
	                  skip_first_id(std::format("{}_{}_{}_skip_first", command.author.user_id.str(), command.channel_id.str(), id)),
	                  skip_last_id(std::format("{}_{}_{}_skip_last", command.author.user_id.str(), command.channel_id.str(), id))
	{
		id++;
	}
	message_paginator(const dpp::message& message, const std::vector<std::string>& messages ,base_command& command): message(message),
	                  messages(messages), command(command), action_row(), use_embeds{false},
	                  forward_id(std::format("{}_{}_{}_forward", command.author.user_id.str(), command.channel_id.str(), id)),
	                  backward_id(std::format("{}_{}_{}_backward", command.author.user_id.str(), command.channel_id.str(), id)),
	                  skip_first_id(std::format("{}_{}_{}_skip_first", command.author.user_id.str(), command.channel_id.str(), id)),
	                  skip_last_id(std::format("{}_{}_skip_last", command.author.user_id.str(), command.channel_id.str(), id))
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
	[[nodiscard]] std::size_t size() const;

	/**
	 * @brief start - Starts the paginator and activates the buttons.
	 */
	void start();

	[[nodiscard]] bool is_embed_paginator() const;

};

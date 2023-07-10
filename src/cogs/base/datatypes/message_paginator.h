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
#include <fmt/format.h>

class message_paginator {
	std::vector<dpp::embed> pages;
	std::array<dpp::component, 4> buttons{};
	dpp::component action_row;
	dpp::message message;
	base_command command;
	ushort current_page{0};
	bool const use_embeds;
	std::vector<std::string> messages;


	void forward();
	void backward();
	void skip_last();
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
	                  forward_id(fmt::format("{}_{}_{}_forward", command.author.user_id, command.channel_id, id)),
	                  backward_id(fmt::format("{}_{}_{}_backward", command.author.user_id, command.channel_id, id)),
	                  skip_first_id(fmt::format("{}_{}_{}_skip_first", command.author.user_id, command.channel_id, id)),
	                  skip_last_id(fmt::format("{}_{}_{}_skip_last", command.author.user_id, command.channel_id, id))
	{
		id++;
	}
	message_paginator(const dpp::message& message, const std::vector<std::string>& messages ,base_command& command): message(message),
	                  messages(messages), command(command), action_row(), use_embeds{false},
	                  forward_id(fmt::format("{}_{}_{}_forward", command.author.user_id, command.channel_id, id)),
	                  backward_id(fmt::format("{}_{}_{}_backward", command.author.user_id, command.channel_id, id)),
	                  skip_first_id(fmt::format("{}_{}_{}_skip_first", command.author.user_id, command.channel_id, id)),
	                  skip_last_id(fmt::format("{}_{}_skip_last", command.author.user_id, command.channel_id, id))
	{
		id++;
	}

	void add_page(const dpp::embed &embed);
	void add_page(const std::string& page);
	[[nodiscard]] std::size_t size() const;
	void start();

	[[nodiscard]] bool is_embed_paginator() const;

};

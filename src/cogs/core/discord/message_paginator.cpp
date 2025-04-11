//
// Created by arshia on 3/1/23.
//

#include "message_paginator.h"

#include <stdexcept>
#include <format>

void message_paginator::add_page(const dpp::embed& embed) {
	if(is_embed_paginator())
		pages.push_back(embed);
	else
		throw std::invalid_argument{"Cannot have a mix of embeds and texts in the paginator."};
}

void message_paginator::add_page(const std::string& page) {
	if(!is_embed_paginator())
		messages.push_back(page);
	else
		throw std::invalid_argument{"Cannot have a mix of embeds and texts in the paginator."};
}

void message_paginator::remove_page(std::size_t page) {
	if(page >= size())
		throw std::out_of_range{std::format("Page {} does not exist in embed. Page number can only be between 0-{}.", page, size())};
	pages.erase(pages.begin() + page);
}

void message_paginator::start() {
	if(is_started)
		return;
	is_started = true;

	buttons[SKIP_FIRST].set_emoji("⏮️")
		.set_id(skip_first_id)
		.set_style(dpp::cos_primary);
	buttons[BACKWARD].set_emoji("◀️")
			.set_id(backward_id).set_style(dpp::cos_primary);
	buttons[FORWARD].set_emoji("▶️").set_id(forward_id).set_style(dpp::cos_primary);
	buttons[SKIP_LAST].set_emoji("⏭️").set_id(skip_last_id).set_style(dpp::cos_primary);

	for (auto const& button: buttons)
		action_row.add_component(button);

	m_message.components.clear();
	m_message.add_component(action_row);

	if (m_command.interaction) {
		(*m_command.interaction)->reply(m_message);
	} else {
		m_command.bot->message_create(m_message);
	}
	m_command.bot->on_button_click([this](const dpp::button_click_t &event) {
		if (event.custom_id == forward_id) {
			forward();
		}
		else if (event.custom_id == backward_id) {
			backward();
		}
		else if (event.custom_id == skip_first_id) {
			skip_first();
		}
		else if (event.custom_id == skip_last_id) {
			skip_last();
		}
	});
}

void message_paginator::start(const user_ptr& user){
	if(is_started)
		return;
	is_started = true;

	buttons[SKIP_FIRST].set_emoji("⏮️").set_id(skip_first_id).set_style(dpp::cos_primary);
	buttons[BACKWARD].set_emoji("◀️").set_id(backward_id).set_style(dpp::cos_primary);
	buttons[FORWARD].set_emoji("▶️").set_id(forward_id).set_style(dpp::cos_primary);
	buttons[SKIP_LAST].set_emoji("⏭️").set_id(skip_last_id).set_style(dpp::cos_primary);

	for (auto const& button: buttons)
		action_row.add_component(button);

	m_message.components.clear();
	m_message.add_component(action_row);

	m_command.bot->direct_message_create(user->id, m_message);

	m_command.bot->on_button_click([this](const dpp::button_click_t &event) {
		if (event.custom_id == forward_id) {
			forward();
		}
		else if (event.custom_id == backward_id) {
			backward();
		}
		else if (event.custom_id == skip_first_id) {
			skip_first();
		}
		else if (event.custom_id == skip_last_id) {
			skip_last();
		}
	});
}

void message_paginator::forward() {
	current_page = ++current_page % size();
	if(is_embed_paginator()) {
		m_message.embeds.clear();
		m_message.add_embed(pages[current_page]);
	}
	else {
		m_message.set_content(messages[current_page]);
	}
	if(m_command.interaction)
		(*m_command.interaction)->edit_response(m_message);
	else
		m_command.bot->message_edit(m_message);
}

void message_paginator::backward() {
	if(current_page == 0)
		current_page = size() - 1;
	else
		current_page--;
	if(is_embed_paginator()) {
		m_message.embeds.clear();
		m_message.add_embed(pages[current_page]);
	}
	else {
		m_message.set_content(messages[current_page]);
	}
	if(m_command.interaction)
		(*m_command.interaction)->edit_response(m_message);
	else
		m_command.bot->message_edit(m_message);
}

void message_paginator::skip_last() {
	current_page = size() - 1;
	if(is_embed_paginator()) {
		m_message.embeds.clear();
		m_message.add_embed(pages[current_page]);
	}
	else {
		m_message.set_content(messages[current_page]);
	}
	if(m_command.interaction)
		(*m_command.interaction)->edit_response(m_message);
	else
		m_command.bot->message_edit(m_message);
}


void message_paginator::skip_first() {
	current_page = 0;
	if(is_embed_paginator()) {
		m_message.embeds.clear();
		m_message.add_embed(pages[current_page]);
	}
	else {
		m_message.set_content(messages[current_page]);
	}
	if(m_command.interaction)
		(*m_command.interaction)->edit_response(m_message);
	else
		m_command.bot->message_edit(m_message);
}
message_paginator::~message_paginator() {
	action_row.components.clear();
	for(auto& button: buttons) {
		button.set_disabled(true);
		action_row.add_component(button);
	}

	m_message.components.clear();
	m_message.add_component(action_row);

	if(m_command.interaction)
		(*m_command.interaction)->edit_response(m_message);
	else
		m_command.bot->message_edit(m_message);
}

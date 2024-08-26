//
// Created by arshia on 8/26/24.
//

#include "commands.h"

namespace reactaio::moderation {
	inline dpp::slashcommand define_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options) {
		dpp::slashcommand command{properties.name, properties.description, properties.bot->me.id};
		for(auto const& option : options)
			command.add_option(option);
		return command;
	}

	inline void define_guild_command(const slash_command_properties &properties, const std::vector<dpp::command_option> &options, dpp::snowflake const &guild_id) {
		dpp::slashcommand command{properties.name, properties.description, properties.bot->me.id};
		for(auto const& option : options)
			command.add_option(option);
		properties.bot->guild_command_create(command, guild_id);
	}
}
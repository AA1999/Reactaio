import json
import sys
import traceback
from asyncio.events import get_event_loop

import discord
from asyncpg.pool import create_pool
from discord.abc import GuildChannel
from discord.channel import TextChannel
from discord.ext.commands.bot import Bot, when_mentioned_or
from discord.guild import Guild
from discordtoken import DISCORD_TOKEN
from pathlib import Path

TOKEN = DISCORD_TOKEN
    
bot_prefix = '.'

intent: discord.Intents = discord.Intents.all()

bot: Bot = Bot(command_prefix=when_mentioned_or(bot_prefix), intents=intent, case_insensitive=True)

POSTGRES_INFO = {
    'user': 'postgres',
    'password': 'root',
    'database': 'Reactaio',
    'host': 'localhost',
}


@bot.event
async def on_ready():
    print(f' {bot.user} ({bot.user.id}) has connected.')


@bot.event
async def on_guild_channel_create(channel: GuildChannel):
    if isinstance(channel, TextChannel):
        bot.channels = {
            channel.name: channel.id for channel in channel.guild.text_channels
        }

        with open('channels.json', mode='w') as f:
            json.dump(bot.channels, f, ensure_ascii=False)

@bot.event
async def on_guild_channel_update(before: GuildChannel, after: GuildChannel):
    if isinstance(after, TextChannel) and before.name != after.name:
        bot.channels = {
            channel.name: channel.id for channel in after.guild.text_channels
        }

        with open('channels.json', mode='w') as f:
            json.dump(bot.channels, f, ensure_ascii=False)

@bot.event
async def on_guild_channel_delete(channel: GuildChannel):
    if isinstance(channel, TextChannel):
        bot.channels = {
            channel.name: channel.id for channel in channel.guild.text_channels
        }

        with open('channels.json', mode='w') as f:
            json.dump(bot.channels, f, ensure_ascii=False)

async def setup():
    await bot.wait_until_ready()
    guild: Guild = bot.get_guild(812314425318440961)
    try:
        with open('channels.json', mode='x') as f:
            bot.channels = {}
            for channel in guild.text_channels:
                bot.channels[channel.name] = channel.id
            json.dump(bot.channels, f, ensure_ascii=False)
    except FileExistsError:
        with open('channels.json', mode='r') as f:
            bot.channels = json.load(f)

loop = get_event_loop()

bot.pool = loop.run_until_complete(create_pool(**POSTGRES_INFO))
bot.loop.create_task(setup())

for fileName in Path('cogs').glob('**/*.py'):
    *tree, _ = fileName.parts
    try:
        bot.load_extension(f"{'.'.join(tree)}.{fileName.stem}")
    except Exception as e:
        traceback.print_exception(type(e), e, e.__traceback__, file=sys.stderr)

bot.run(TOKEN)

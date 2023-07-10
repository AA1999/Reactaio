from aiohttp import ClientSession
from discord import Asset, TextChannel
from discord.emoji import Emoji
from discord.ext import commands
from discord.ext.commands import Bot, Cog, Context
from discord.ext.commands.core import command, has_permissions
from discord.guild import Guild
from discord.partial_emoji import PartialEmoji


class Emoji(Cog):

    def __init__(self, bot: Bot):
        self.bot: Bot = bot

    @has_permissions(manage_emojis=True)
    @command(aliases=['steal', 'addemote', 'add'])
    async def addemoji(self, ctx: Context, emoji, *, name: str = None):
        emojichl: TextChannel = ctx.guild.get_channel(self.bot.channels['emoji-bot'])
        if ctx.channel != emojichl:
            return
        if name is None and isinstance(emoji, PartialEmoji) or name is None and isinstance(emoji, Emoji):
            name = emoji.name
        elif name is None:
            await ctx.send('Please provide a name for the emoji.')
            return
        guild: Guild = ctx.guild
        if isinstance(emoji, PartialEmoji):
            result: Emoji = await guild.create_custom_emoji(name=name,  image=await emoji.url.read())
        else:
            async with ClientSession() as session:
                async with session.get(emoji) as request:
                    result: Emoji = await guild.create_custom_emoji(name=name, image=await request.read())
        await ctx.send(f'Added emoji {str(result)} with the name {name}')

    @commands.has_permissions(manage_emojis=True)
    @commands.command(aliases = ['del', 'delemote'])
    async def deleteemoji(self, ctx: Context, emoji: Emoji):
        if emoji.guild == ctx.guild:
            await ctx.send(f'Successfully deleted the custom emoji {str(emoji)}.')
            await emoji.delete()
        else:
            await ctx.send('Emoji does not belong to the guild.')


def setup(client: Bot):
    client.add_cog(Emoji(client))


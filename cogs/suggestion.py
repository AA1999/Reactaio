from datetime import datetime

from asyncpg import Connection
from discord import TextChannel, Message, Embed
from discord.ext.commands.cog import Cog
from discord.ext.commands.bot import Bot
from discord.ext.commands import command
from discord.ext.commands import Context


class Suggestion(Cog):
    bot: Bot

    @Cog.listener()
    async def on_message(self, message: Message):
        if message.guild is None:
            return
        suggestions: TextChannel = message.guild.get_channel(self.bot.channels['suggest-something'])
        gsuggestions: TextChannel = message.guild.get_channel(self.bot.channels['suggest-games'])
        if message.channel == suggestions or message.channel == gsuggestions:
            await message.delete()

    def __init__(self, bot: Bot):
        self.bot = bot

    @command()
    async def suggest(self, ctx: Context, *, suggestion: str):
        await self.bot.wait_until_ready()
        if ctx.guild is None:
            return
        suggestions: TextChannel = ctx.guild.get_channel(self.bot.channels['suggest-something'])
        destination: TextChannel = ctx.guild.get_channel(self.bot.channels['suggestions'])
        suggest_games: TextChannel = ctx.guild.get_channel(self.bot.channels['suggest-games'])
        gsuggestions: TextChannel = ctx.guild.get_channel(self.bot.channels['game-suggestions'])

        if ctx.channel not in [suggestions, suggest_games]:
            return

        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                sid = await connection.fetchval('SELECT id FROM suggestions ORDER BY id DESC LIMIT 1')
                if sid is None:
                    sid = 1
                else:
                    sid += 1
                msg: Embed = Embed(title=f'Suggestion #{sid}', color=0x00ffcc, timestamp=datetime.utcnow())
                msg.description = suggestion
                msg.set_footer(text=f'{ctx.author.display_name}#{ctx.author.discriminator}',
                               icon_url=ctx.author.avatar_url)
                if ctx.channel == suggestions:
                    message: Message = await destination.send(embed=msg)
                else:
                    message: Message = await gsuggestions.send(embed=msg)

                await message.add_reaction('<:yes:813373603239559218>')
                await message.add_reaction('<:no:813373661276143626>')

                await connection.execute('INSERT INTO suggestions (authorid, datetimemade) VALUES ($1, $2)', ctx.author.id, ctx.message.created_at, )


def setup(bot: Bot):
    bot.add_cog(Suggestion(bot))

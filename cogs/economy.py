from datetime import datetime
from typing import Optional

from asyncpg import Connection
from discord import Guild, Member, Message, Embed
from discord.ext.commands import Cog, Bot, command, Context


class Economy(Cog):
    bot: Bot

    def __init__(self, bot: Bot):
        self.bot = bot
        self.bot.loop.create_task(self.config())

    async def config(self):
        await self.bot.wait_until_ready()
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                guild: Guild = self.bot.get_guild(812314425318440961)
                member: Member
                for member in guild.members:
                    if not member.bot:
                        await connection.execute('INSERT INTO economy (memberid, hearts) VALUES ($1, $2) ON CONFLICT '
                                                 'DO NOTHING ', member.id, 1000, )

    @command(aliases=['bal'])
    async def balance(self, ctx: Context, member: Optional[Member]):
        if member is None:
            member = ctx.author
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                bal = await connection.fetchval('SELECT hearts FROM economy WHERE memberid = $1', member.id, )
                result: Embed = Embed(title=f'{member.display_name}#{member.discriminator}\'s Balance', color=0x00ff00,
                                      timestamp=datetime.utcnow())
                result.description = f'{str(member)} has {bal} hearts.'
                await ctx.send(embed=result)


def setup(bot: Bot):
    bot.add_cog(Economy(bot))

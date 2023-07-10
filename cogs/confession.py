from datetime import datetime

from asyncpg.connection import Connection
from discord.channel import TextChannel
from discord.colour import Colour
from discord.embeds import Embed
from discord.ext.commands import Bot, Cog, Context
from discord.ext.commands.core import command, dm_only, has_any_role
from discord.guild import Guild
from discord.member import Member


class Confession(Cog):

    bot: Bot

    def __init__(self, bot: Bot) -> None:
        self.bot = bot
    
    @command()
    @dm_only()
    async def confess(self, ctx: Context, *, confession: str):
        confessions: TextChannel = await self.bot.fetch_channel(812544316496019456)
        confess: Embed = Embed(timestamp = datetime.utcnow(), color = Colour.random())
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                ismuted = connection.fetchval('SELECT memberid FROM confessionmutes WHERE memberid = $1', (ctx.author.id), )
                if ismuted == ctx.author.id:
                    await ctx.send('You\'re not allowed to post confessions because of mute punishment.')
                else:
                    id = await connection.fetchval('SELECT id FROM confessions ORDER BY id DESC LIMIT 1')
                    if id is None:
                        id = 1
                    else:
                        id += 1
                    guild: Guild = await self.bot.fetch_guild(812314425318440961)
                    confess.title = f'Confession #{id}: '
                    confess.set_author(name = 'Anonymous Sinner', icon_url = guild.icon_url)
                    confess.set_footer(text=f'DM the bot with {self.bot.command_prefix}confess \'your confession\' to confess anonymously.')
                    confess.description = confession
                    await confessions.send(embed = confess)
                    await ctx.send(f'Confession #{id} is now in {confessions.mention}.')
                    await connection.execute('INSERT INTO confessions (id, authorid, datetimemade) VALUES ($1, $2, $3)', id, ctx.author.id, datetime.utcnow(), )
    
    @has_any_role('Admin', 'Head Admin', 'Owner')
    @command(aliases=['cmute'])
    async def mute_confession(self, ctx: Context, id: int, *, reason: str = None):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    user = await connection.fetchval('SELECT authorid FROM confessions WHERE id = $1', (id), )
                    await connection.execute('INSERT INTO confessionmutes (memberid, datetimemuted, reason) VALUES($1, $2, $3)', user, datetime.utcnow(), reason, )
                    embed: Embed = Embed(title = 'Muted', color = 0xff0000, description = f'Confession #{id} was successfuly muted. \nAuthor has been DMed about the mute', timestamp = datetime.utcnow())
                    dm: Embed = Embed(title = 'Muted', color = 0xff0000, description = f'You\'ve been muted from confessing in {ctx.guild.name}. **Rason**: {reason}\n', footer = 'Contact an administrator for appealing the mute.')
                    member: Member = await ctx.guild.fetch_member(user)
                    await member.send(embed = dm)
                    await ctx.send(embed = embed)
                except:
                    await ctx.send('Invalid id. Please try again.\n')
    
    @has_any_role('Admin', 'Head Admin', 'Owner', 'Grand Inquisitor')
    @command(aliases = ['cunmute'])
    async def unmute_confession(self, ctx: Context, member: Member, *, reason: str = None):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    await connection.fetchval('DELETE FROM confessionmutes WHERE memberid = $1', (member.id), )
                    embed: Embed = Embed(title = 'Muted', color = 0xff0000, description = f'Confession for {str(member)} was successfuly unmuted. \nUser has been DMed about the unmute', timestamp = datetime.utcnow())
                    dm: Embed = Embed(title = 'Unmuted', color = 0xff0000, description = f'You\'ve been unmuted from confessing in {ctx.guild.name}. **Rason**: {reason}\n', footer = 'Please behave better in next confessions.')
                    await member.send(embed = dm)
                    await ctx.send(embed = embed)
                except:
                    await ctx.send('Invalid or already unmuted member. Please try again.\n')

    @command()
    @has_any_role('Owner', 'Head Admin')
    async def reveal(self, ctx: Context, id: int):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    user = await connection.fetchval('SELECT authorid FROM confessions WHERE id = $1', (id), )
                    member: Member = await ctx.guild.fetch_member(user)
                    embed: Embed = Embed(title = 'Reveal', color = 0xff0000, description = f'Confession #{id}\'s author is: **{str(member)}**.', timestamp = datetime.utcnow())
                    await ctx.send(embed = embed)
                except:
                    await ctx.send('Invalid id. Please try again.\n')          

def setup(bot: Bot):
    bot.add_cog(Confession(bot))

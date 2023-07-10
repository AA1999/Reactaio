import asyncio
from datetime import datetime
from typing import Union

import discord
from asyncpg.connection import Connection
from discord import VoiceChannel, Role, Guild, User
from discord.colour import Colour
from discord.embeds import Embed
from discord.errors import Forbidden, HTTPException
from discord.ext import commands
from discord.ext.commands import Cog
from discord.ext.commands.bot import Bot
from discord.ext.commands.context import Context
from discord.ext.commands.core import command
from discord.ext.tasks import loop
from discord.member import Member
from discord.message import Message


def human(guild: Guild):
    member: Member
    count: int = 0
    for member in guild.members:
        if not member.bot:
            count += 1
    return count


def robots(guild: Guild):
    member: Member
    count: int = 0
    for member in guild.members:
        if member.bot:
            count += 1
    return count


class Info(commands.Cog):
    bot: Bot

    @loop(minutes=10)
    async def update(self):
        await self.bot.wait_until_ready()
        guild: Guild = self.bot.get_guild(812314425318440961)
        humans: VoiceChannel = self.bot.get_channel(824184520948318228)
        total: VoiceChannel = self.bot.get_channel(838754323433390090)
        bots: VoiceChannel = self.bot.get_channel(824184541794009108)
        online: VoiceChannel = self.bot.get_channel(824184588242255892)
        peak: VoiceChannel = self.bot.get_channel(824184633154994176)

        await online.edit(
            name=f'Online Members: {len(list(filter(lambda m: str(m.status) == "online", online.guild.members)))}')
        with open('peak.data', mode='r') as f:
            line = f.readline()
            peakon = int(line)
            peakon = max(
                peakon,
                len(
                    list(
                        filter(
                            lambda m: str(m.status) == "online",
                            online.guild.members,
                        )
                    )
                ),
            )
        with open('peak.data', mode='w') as f:
            f.write(str(peakon))

        members = human(guild)
        bot = robots(guild)
        await humans.edit(name=f'Human Members: {members}')
        await total.edit(name=f'Total Members: {guild.member_count}')
        await bots.edit(name=f'Bot Members: {bot}')
        await peak.edit(name=f'Peak Online Members: {peakon}')

    class AFK:
        member: discord.Member
        reason: str

        def __init__(self, member: discord.Member, reason: str):
            self.member = member
            self.reason = reason

    def __init__(self, bot: Bot):
        self.bot: Bot = bot
        self.update.start()

    @command(aliases=['test'])
    async def ping(self, ctx: Context):
        await ctx.send(f'Pong! {int(self.bot.latency * 1000)}ms')

    @command()
    async def humans(self, ctx: commands.Context):
        members = ctx.guild.members
        count = sum(not member.bot for member in members)
        await ctx.send(f'{ctx.guild.name} has {count} non-bot members.')

    @command(aliases=['av'])
    async def avatar(self, ctx: Context, member: Member = None):
        if member is None:
            member = ctx.author
        av: Embed = Embed(title=f'{str(member)}\'s Avatar', color=Colour.random(), timestamp=datetime.utcnow())
        av.set_image(url=member.avatar_url)
        await ctx.send(embed=av)

    @command(aliases=['robots'])
    async def bots(self, ctx: commands.Context):
        members = ctx.guild.members
        count = sum(1 for member in members if member.bot)
        await ctx.send(f'{ctx.guild.name} has {count} bots.')

    @command(aliases=['info', 'memberinfo', 'lookup', 'search'])
    async def whois(self, ctx: Context, member: Union[Member, User, None]):
        if member is None:
            member = ctx.author
        info: Embed = Embed(title=f'{str(member)}\'s info', color=Colour.random())
        info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        acknowledgements: str
        if (isinstance(member, Member)):
            if member.guild.owner == member:
                acknowledgements = 'Server Owner'
            elif member.guild_permissions.administrator == True:
                acknowledgements = 'Server Administrator'
            elif member.guild_permissions.manage_guild == True:
                acknowledgements = 'Server Manager'
            elif member.guild_permissions.ban_members == True or member.guild_permissions.kick_members == True:
                acknowledgements = 'Server Moderator'
            else:
                acknowledgements = 'Server Member'
            roleinfo: list[str] = []
            role: Role
            for role in member.roles:
                if role == member.guild.default_role:
                    continue
                roleinfo.append(f'{role.mention}, ')
            roleinfo.reverse()
            roles: str = ''.join(roleinfo)
            roles = roles[:-2]
            info.add_field(name='Nickname:', value=str(member), inline=False)
            info.add_field(name='Joined at:', value=member.joined_at, inline=False)
            info.add_field(name=f'Roles ({len(member.roles)}):',
                           value=roles if len(roles) < 1024 else 'Too many to show',
                           inline=False)
            if member.premium_since is not None:
                info.add_field(name='Premium Since: ', value=member.premium_since, inline=False)
            info.add_field(name='Permissions: ', value=member.guild_permissions, inline=False)
            info.add_field(name='Acknowledgements: ', value=acknowledgements, inline=False)
        info.set_author(name=str(member), icon_url=member.avatar_url)
        info.add_field(name='Created at:', value=member.created_at, inline=False)
        info.set_thumbnail(url=member.avatar_url)
        if isinstance(member, Member):
            info.set_footer(text='This user is a member of this guild.')
        else:
            info.set_footer(text='This user is not a member of this guild.')
        await ctx.send(embed=info)

    @command()
    async def afk(self, ctx: commands.Context, *, reason: str = None):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                nick = ctx.author.display_name
                await connection.execute(
                    'INSERT INTO afks (memberid, displayname, afkdatetime, reason) VALUES ($1, $2, $3, $4)',
                    ctx.author.id, nick, datetime.utcnow(), reason, )
                try:
                    nick = f'[AFK] {nick}'
                    await ctx.author.edit(nick=nick)
                except Forbidden:
                    pass
                except HTTPException:
                    pass
                await ctx.send(
                    f'{ctx.author.mention}, I set your AFK. {f"Reason: {reason}" if reason is not None else ""}')

    @Cog.listener()
    async def on_message(self, message: Message):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                afks = await connection.fetch('SELECT memberid, afkdatetime, reason FROM afks')
                for afk in afks:
                    member: Member = await message.guild.fetch_member(afk['memberid'])
                    time: datetime = afk['afkdatetime']
                    reason: str = afk['reason']
                    if (
                        member.mentioned_in(message)
                        and not message.author.bot
                        and message.author.id != member.id
                    ):
                        await message.channel.send(
                            f'{member.mention} is AFK since {time}.{f"Reason: {reason}" if reason is not None else ""}')
                    if message.author == member:
                        back: Message = await message.channel.send(
                            f'Welcome back, {member.mention}! I removed your AFK')
                        try:
                            displayname: str = await connection.fetchval(
                                'SELECT displayname FROM afks WHERE memberid = $1', (member.id), )
                            await member.edit(nick=displayname)
                        except Forbidden:
                            pass
                        await asyncio.sleep(5)
                        await back.delete()
                        await connection.execute('DELETE FROM afks WHERE memberid = $1', (member.id), )


def setup(bot: Bot):
    bot.add_cog(Info(bot))

import datetime
import re
from datetime import datetime, timedelta
from typing import Union

import discord
from asyncpg.connection import Connection
from discord import User, Colour, AuditLogEntry, AuditLogAction
from discord.channel import TextChannel
from discord.embeds import Embed
from discord.errors import Forbidden, HTTPException, NotFound
from discord.ext import commands, tasks
from discord.ext.commands import Paginator
from discord.ext.commands.bot import Bot
from discord.ext.commands.cog import Cog
from discord.ext.commands.context import Context
from discord.ext.commands.converter import Greedy
from discord.ext.commands.core import command, has_permissions, has_guild_permissions, has_any_role, group
from discord.ext.tasks import loop
from discord.guild import Guild, BanEntry
from discord.iterators import AuditLogIterator
from discord.member import Member
from discord.message import Message, MessageReference, DeletedReferencedMessage
from discord.utils import get


class Moderation(Cog):
    bot: commands.Bot
    id: int

    def server_name(self, member: Union[Member, User]):
        return f'{member.display_name}#{member.discriminator}'

    def __init__(self, bot: Bot):
        self.bot = bot
        with open('log.data', mode='r') as f:
            line = f.readline()
            self.id = int(line) + 1
        self.tempbancheck.start()
        self.tempmutecheck.start()

    @loop(seconds=5.0)
    async def tempbancheck(self):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                rows = await connection.fetch('SELECT "end", userid FROM tempbans')
                if rows is not None:
                    for row in rows:
                        try:
                            if row['end'] <= datetime.utcnow():
                                mid = row['userid']
                                user = await self.bot.fetch_user(mid)
                                guild: Guild = self.bot.get_guild(id=812314425318440961)
                                if user is not None:
                                    await guild.unban(user=user, reason='End of ban duration.')
                                await connection.execute('DELETE FROM "tempbans" WHERE "end" = $1', row['end'], )
                        except HTTPException:
                            pass

    @tempbancheck.before_loop
    async def before_tempbancheck(self):
        await self.bot.wait_until_ready()

    @loop(seconds=5.0)
    async def tempmutecheck(self):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                rows = await connection.fetch('SELECT "end", memberid FROM tempmutes')
                if rows is not None:
                    for row in rows:
                        try:
                            if row['end'] <= datetime.utcnow():
                                guild: Guild = self.bot.get_guild(id=812314425318440961)
                                muted = get(guild.roles, name='Muted')
                                id = row['memberid']
                                member: Member = await guild.fetch_member(id)
                                if member is not None:
                                    await member.remove_roles(muted)
                                await connection.execute('DELETE FROM tempmutes WHERE "end" = $1', row['end'], )
                        except HTTPException:
                            pass

    @tempmutecheck.before_loop
    async def before_tempmutecheck(self):
        await self.bot.wait_until_ready()

    @command()
    @has_permissions(kick_members=True)
    async def kick(self, ctx: Context, member: Member, *, reason=None):
        if ctx.guild is None:
            return
        if ctx.author.top_role <= member.top_role:
            await ctx.send('You\'re not allowed to kick a member with a higher than or equal role from you.')
            return
        embed = Embed(title='Kicked', color=0x00ffcc)
        embed.set_author(name=ctx.author)
        embed.set_image(url='https://media.tenor.com/images/ff05d3c458a2e8ca1110a1b92a3a9326/tenor.gif')
        embed.set_thumbnail(url=ctx.guild.icon_url)
        embed.description = f'**{self.server_name(member)}** was kicked from {ctx.guild.name}.' \
                            f'Reason: {reason}'

        try:
            dm: Embed = Embed(title='Kicked', color=0xff0000, timestamp=datetime.utcnow(),
                              description=f'You were kicked from {ctx.guild.name}.')
            dm.add_field(name='Moderator: ', value=self.server_name(ctx.author), inline=True)
            dm.add_field(name='Reason: ', value=reason, inline=True)
            await member.send(embed=dm)
        except Forbidden:
            pass
        except HTTPException:
            pass
        await member.kick(reason=reason)
        await ctx.send(embed=embed)
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} kicked {member.mention} ({member.id}) from **{ctx.guild.name}**. ' \
                          f'Reason: {reason}.'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(member)}', icon_url=member.avatar_url)
        await hallofshame.send(embed=log)
        await modlog.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @command()
    @has_permissions(ban_members=True)
    async def ban(self, ctx: Context, user: Union[Member, User] = None, duration: str = None, *, reason=None):
        if ctx.guild is None:
            return
        if user is None:
            embed: Embed = Embed(title='Ban', color=0x00ffaa, thumbnail=ctx.guild.banner_url)
            embed.description = f'{self.bot.command_prefix}ban [user] [duration] [reason]\n'
            embed.add_field(name='Example', value=f'{self.bot.command_prefix}ban 1230231032 10m very cool')
            await ctx.send(embed=embed)
            return
        if isinstance(user, Member):
            if ctx.author.top_role <= user.top_role:
                await ctx.send('You\'re not allowed to ban a member with a higher than or equal role from you.')
                return
        embed = Embed(title='Banned', color=0x00ffaa, thumbnail=ctx.guild.banner_url)
        d = re.findall(pattern='(\d+)( )?(d|h|m|y|days|hours|minutes|years|day|hour|minute|year)[ |\s|\n]',
                       string=f'{duration} ')
        if d:
            duration = d[0][0]
            prefix = d[0][-1]
            if prefix in ['y', 'year', 'years']:
                hours = int(duration) * 365 * 24
                val = f' for {duration} years.'
            elif prefix in ['m', 'month', 'months']:
                hours = int(duration) * 30 * 24
                val = f' for {duration} months.'
            elif prefix in ['d', 'day', 'days']:
                hours = int(duration) * 24
                val = f' for {duration} days.'
            elif prefix in ['h', 'hour', 'hours']:
                hours = int(duration)
                val = f' for {duration} hours.'
        else:
            val = ''
            reason = f'{duration} {reason}' if reason is not None else duration
        embed.description = f'**{str(user)}** was banned from {ctx.guild.name}{val}. reason: {reason}'
        embed.set_image(url='https://media3.giphy.com/media/LOoaJ2lbqmduxOaZpS/giphy.gif')
        embed.set_thumbnail(url='https://media.tenor.com/images/181fd3b2bcb8c61f2eeda4d5f3d3fd3b/tenor.gif')
        embed.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        await ctx.guild.ban(user=user, delete_message_days=0, reason=reason)
        await ctx.send(embed=embed)
        try:
            await user.send(embed=Embed(title='Banned', description=f'You were banned from {ctx.message.guild}{val}.'
                                                                    f' reason: {reason}. Moderator: {ctx.author}'))
        except Forbidden:
            pass
        except HTTPException:
            pass
        if 'hours' in locals():
            now: datetime = datetime.utcnow()
            delta: timedelta = timedelta(hours=int(hours))
            end: datetime = now + delta
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    await connection.execute('INSERT INTO tempbans (userid, "end", reason) VALUES ($1, $2, $3) '
                                             'ON CONFLICT(userid) DO UPDATE SET "end" = $2', user.id, end, reason, )
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        bans: list[BanEntry] = await ctx.guild.bans()
        log: Embed = Embed(title=f'Ban Entry #{len(bans)}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} banned {user.mention} ({user.id}){val}. Reason: {reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await hallofshame.send(embed=log)
        await modlog.send(embed=log)

    @command()
    @has_permissions(ban_members=True)
    async def rban(self, ctx: Context, duration: str = None, *, reason: str = None):
        message: Message = ctx.message

        r: MessageReference = message.reference
        if r is None:
            await ctx.send('This command needs to be used as a reply to the author of the non-deleted message.')
            return
        ref: Message = r.resolved
        user: Union[User, Member] = ref.author
        await self.ban(ctx=ctx, user=user, duration=duration, reason=reason)


    @command(aliases=['mb'])
    @has_permissions(ban_members=True)
    async def massban(self, ctx: Context, users: Greedy[Union[User, Member]], *, reason: str = None):
        if ctx.guild is None:
            return
        lst: list[str] = []
        for user in users:
            if isinstance(user, Member):
                if ctx.author.top_role <= user.top_role:
                    await ctx.send('You\'re not allowed to ban a member with a higher than or equal role from you.')
                    return
            lst.append(f'**{self.server_name(user)}**')
            lst.append(', ')
        lst.pop()
        names = ''.join(lst)
        embed = Embed(title='Mass Banned', color=0x00ffaa, thumbnail=ctx.guild.banner_url)
        embed.description = f'{names} were banned from {ctx.guild.name}. reason: {reason}'
        embed.set_image(url='https://i.gifer.com/1Daz.gif')
        embed.set_thumbnail(
            url='https://64.media.tumblr.com/cdcd9da6ba3de2d0a670b9b68b0e98bb/tumblr_nq42q1aXDb1tslewgo1_500.gifv')
        embed.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        await ctx.send(embed=embed)
        for user in users:
            await ctx.guild.ban(user=user, delete_message_days=0, reason=reason)
            try:
                if isinstance(user, Member):
                    await user.send(embed=Embed(title='Banned',
                                                description=f'You were banned from **{ctx.message.guild}**. Reason: '
                                                            f'{reason}.'
                                                            f'Moderator: **{ctx.author}**'))
            except Forbidden:
                pass

        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        log: Embed = Embed(title=f'Ban #{len(await ctx.guild.bans())}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} banned {names} from **{ctx.guild.name}**. Reason: {reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)

    @command(aliases=['strike'])
    @has_guild_permissions(mute_members=True)
    async def warn(self, ctx: commands.Context, member: discord.Member, *, reason: str):
        if ctx.guild is None:
            return
        if ctx.author.top_role <= member.top_role:
            await ctx.send('You\'re not allowed to warn a member with a higher than or equal role from you.')
            return
        warning: Embed = Embed(title='Warning', color=0xff0000,
                               description=f'You\'ve been warned in {ctx.guild}. reason: {reason}. Moderator: '
                                           f'**{ctx.author}**')
        warning.set_footer(text=member.guild.name)
        report: Embed = Embed(title='Warning', color=0xff0000,
                              description=f'**{str(member)}** has been warned. reason: {reason}. Moderator: '
                                          f'**{ctx.author}**')
        report.set_footer(text=member.guild.name)
        await ctx.send(embed=report)
        try:
            await member.send(embed=warning)
        except Forbidden:
            pass
        except HTTPException:
            pass
        warnid: int
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                warnid = await connection.fetchval('INSERT INTO warnings (memberid, warnedat, reason) VALUES ($1, $2, '
                                                   '$3) RETURNING id', member.id, datetime.utcnow(), reason, )
        if warnid is None:
            return
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        log: Embed = Embed(title=f'Warning #{warnid}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has warned {member.mention} ({member.id}). Reason: {reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)

    @command()
    @has_guild_permissions(mute_members=True)
    async def reason(self, ctx: Context, warnid: int, reason: str = None):
        if reason is None:
            await ctx.send('You need to specify a reason.')
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    await connection.execute('UPDATE warnings SET reason = $1 WHERE id = $2', reason, warnid, )
                    await ctx.send(f'Successfully updated reason for warning #{warnid}.')
                except:
                    await ctx.send('Warning not found.')

    @command(aliases=['strikes', 'warns'])
    @has_permissions(kick_members=True)
    async def warnings(self, ctx: Context, member: Member = None):
        if ctx.guild is None:
            return
        if member is None:
            member = ctx.author
        if ctx.author.top_role <= member.top_role:
            await ctx.send('You\'re not allowed to view warnings for member with a higher than or equal role from you.')
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                count = await connection.fetchval('SELECT COUNT(id) FROM warnings WHERE memberid = $1', member.id, )
                print(f'Count is: {count}\n')
                if (count == 0):
                    await ctx.send(f'User **{self.server_name(member)}** has no Warnings.')
                else:
                    warns = await connection.fetch('SELECT * FROM warnings WHERE memberid = $1', member.id, )
                    warnings: Embed = Embed(title=f'{member}\'s Warnings', color=0x00ffaa,
                                            timestamp=datetime.utcnow())
                    warnings.set_footer(text=ctx.guild.name)
                    for warn in warns:
                        warnings.add_field(name='Warning: ', value=warn['id'], inline=False)
                        warnings.add_field(name='Member:', value=self.server_name(member), inline=True)
                        warnings.add_field(name='Warned at:', value=warn['warnedat'], inline=True)
                        warnings.add_field(name='Reason:', value=warn['reason'], inline=True)
                    await ctx.send(embed=warnings)

    @command(aliases=['delwarn', 'delstrike', 'deletestrike', 'deletewarn'])
    @has_guild_permissions(mute_members=True)
    async def deletewarning(self, ctx: commands.Context, wid: int, *, reason: str = None):
        if ctx.guild is None:
            return
        try:
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    mid = await connection.fetchval('SELECT memberid FROM warnings WHERE id = $1', wid, )
                    if mid is None:
                        await ctx.send('Warning not found, please try again with another id.')
                        return
                    member: Member = ctx.guild.get_member(mid)
                    if ctx.author.top_role <= member.top_role:
                        await ctx.send(
                            'You\'re not allowed to delete a warning for a member with a higher than or equal role '
                            'from you.')
                        return
                    if mid == ctx.author.id:
                        await ctx.send('You are unable to delete your own warning.')
                        return
                    await connection.execute('DELETE FROM warnings WHERE id = $1', wid, )
                    await ctx.send("Warning successfully removed.")
                    await member.send(f'Warning #{wid} removed. Reason: {reason}. Moderator: '
                                      f'{self.server_name(ctx.author)}')
                    modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
                    hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
                    log: Embed = Embed(title=f'Warning #{wid} delete: ', color=0x00ff00, timestamp=datetime.utcnow())
                    log.description = f'{ctx.author.mention} removed warn #{wid}. Reason: {reason}'
                    log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
                    log.set_author(name=f'{self.server_name(ctx.author)}',
                                   icon_url=ctx.author.avatar_url)
                    await modlog.send(embed=log)
                    await hallofshame.send(embed=log)
        except:
            await ctx.send('Unexpected error occurred.')

    @command()
    @has_any_role('Owner', 'Admin', 'Head Admin')
    async def lock(self, ctx: Context, channel: TextChannel = None, *, reason: str = None):
        if ctx.guild is None:
            return
        if channel is None:
            channel = ctx.channel
        await channel.set_permissions(channel.guild.default_role, view_channel=True, attach_files=False,
                                      send_messages=False, embed_links=False, add_reactions=False)
        embed: Embed = Embed(title='Locked', timestamp=datetime.utcnow(),
                             description=f'This channel is locked by {ctx.author.mention}, please wait until an '
                                         'Administrator or the Owner unlocks the channel.',
                             color=0xff0000)
        await channel.send(embed=embed)
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        log: Embed = Embed(title=f'Log Entry #{self.id}:', color=0xff0000, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has locked channel {channel.mention} ({channel.id}). Reason: {reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @command()
    @has_any_role('Owner', 'Admin', 'Head Admin')
    async def unlock(self, ctx: Context, channel: TextChannel = None, *, reason: str = None):
        if ctx.guild is None:
            return
        if channel is None:
            channel = ctx.channel
        await channel.set_permissions(channel.guild.default_role, view_channel=True, send_messages=True,
                                      attach_files=False, embed_links=True, add_reactions=True)
        embed: Embed = Embed(title='Unlocked', timestamp=datetime.utcnow(),
                             description=f'This channel is unlocked by {ctx.author.mention}.', color=0xff0000)
        await channel.send(embed=embed)
        modlog: TextChannel = ctx.guild.get_channel(814202335525208124)
        log: Embed = Embed(title=f'Log Entry #{self.id}:', color=0xff0000, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has locked channel {channel.mention} ({channel.id}). Reason: {reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await modlog.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @command(aliases=['uban', 'removeban'])
    @has_permissions(ban_members=True)
    async def unban(self, ctx: Context, user: User, *, reason: str = None):
        if ctx.guild is None:
            return
        await ctx.guild.unban(discord.Object(id=user.id))
        try:
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    await connection.execute('DELETE FROM tempbans WHERE userid = $1', user.id, )
        except:
            pass
        await ctx.send(f'**{str(user)}** was unbanned.')
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        log: Embed = Embed(title=f'Log Entry #{self.id}:', color=0xff0000, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has unbanned {self.server_name(user)}. Reason: ' \
                          f'{reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @command()
    @has_guild_permissions(mute_members=True)
    async def mute(self, ctx: commands.Context, member: discord.Member, duration, *, reason: str = None):
        if ctx.guild is None:
            return
        muted = member.guild.get_role(813044362916266005)
        await member.add_roles(muted, reason=reason)
        d = re.findall('(\d+)( )?(d|h|m|y|days|hours|minutes|years|day|hour|minute|year)[ |\s]', string=f'{duration} ')
        if d != []:
            duration = d[0][0]
            prefix = d[0][-1]
            if prefix in ['y', 'year', 'years']:
                minutes = int(duration) * 365 * 24 * 60
                val = f' for {duration} years.'
            elif prefix in ['m', 'min', 'minute', 'minutes']:
                minutes = int(duration)
                val = f' for {duration} minutes.'
            elif prefix in ['d', 'day', 'days']:
                minutes = int(duration) * 24
                val = f' for {duration} days.'
            elif prefix in ['h', '', 'hour', 'hours']:
                minutes = int(duration) * 60
                val = f' for {duration} hours.'
        else:
            val = ''
            reason = f'{duration} {reason}' if reason is not None else duration
        description = f"**{member}** was muted by **{ctx.message.author}**{val}. Reason: {reason}"
        try:
            dm: Embed = Embed(title='Muted', color=0x00ffff,
                              description=f'You were muted in {ctx.message.guild}{val}. ')
            dm.add_field(name='Reason:', value=reason, inline=True)
            dm.add_field(name='Moderator:', value=ctx.message.author, inline=True)
            await member.send(embed=dm)
        except Forbidden:
            pass
        except HTTPException:
            pass
        embed = Embed(title="Muted", description=description, color=0x00ffaa)
        await ctx.send(embed=embed)
        if 'minutes' in locals():
            now: datetime = datetime.utcnow()
            delta: timedelta = timedelta(minutes=float(minutes))
            end: datetime = now + delta
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    await connection.execute(
                        'INSERT INTO tempmutes (memberid, "end", reason) VALUES($1, $2, $3) ON CONFLICT(memberid) '
                        'DO UPDATE SET "end" = $2', member.id, end, reason, )
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        log: Embed = Embed(title=f'Log Entry #{self.id}:', color=0xff0000, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has muted {member.mention}{val} ({member.id}). Reason: {reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @command()
    @has_permissions(manage_messages=True)
    async def unmute(self, ctx: Context, member: Member, *, reason=None):
        if ctx.guild is None:
            return
        role = get(member.guild.roles, name='Muted')
        await member.remove_roles(role)
        description = f"**{member}** was umuted by **{ctx.message.author}**. reason: {reason}"
        embed = Embed(title="Unmuted", description=description, color=0x00ffaa)
        await member.send(embed=Embed(title='Unmuted', author=ctx.message.author,
                                      description=f'You were unmuted in {ctx.message.guild}. reason: {reason}'))
        await ctx.send(embed=embed)
        try:
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    await connection.execute('DELETE FROM tempmutes WHERE memberid = $1', (member.id,))
        except:
            pass
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        log: Embed = Embed(title=f'Log Entry #{self.id}:', color=0xff0000, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has unmuted {self.server_name(member)} ' \
                          f'({member.id}). Reason: {reason}'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @has_any_role('Owner', 'Head Admin', 'Head Mod', 'Moderator')
    @command()
    async def bans(self, ctx: commands.Context):
        if ctx.guild is None:
            return
        bans = await ctx.guild.bans()
        paginator: Paginator = Paginator(prefix='', suffix='')
        ban: BanEntry
        for ban in bans:
            paginator.add_line(f'**{ban.user}**. Reason: **{ban.reason}**')
        for page in paginator.pages:
            await ctx.send(page)

    def isntpinned(self, message: Message):
        return message not in self.pins

    @command(aliases=['clear', 'clean'])
    @has_permissions(manage_messages=True)
    async def purge(self, ctx: Context, count=10):
        if ctx.guild is None:
            return
        self.pins = await ctx.channel.pins()
        messages: list[Message] = await ctx.channel.purge(limit=count + 1, check=self.isntpinned)
        messagelog: TextChannel = ctx.guild.get_channel(self.bot.channels['message-log'])
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has deleted {count + 1} messages.'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        log.set_author(name=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
        await messagelog.send(embed=log)
        for message in messages:
            msg: Embed = Embed(title=f'Message {message.id}: ', color=0xff0000, timestamp=datetime.utcnow())
            msg.description = message.content if message.content is not None else message.embeds[0].description
            msg.set_footer(text=f'{self.server_name(ctx.author)}', icon_url=ctx.author.avatar_url)
            msg.set_author(name=message.author, icon_url=message.author.avatar_url)
            await messagelog.send(embed=msg)

    @command()
    @has_guild_permissions(manage_channels=True)
    async def nuke(self, ctx: Context, *, reason: str = None):
        modlog: TextChannel = ctx.guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = ctx.guild.get_channel(self.bot.channels['🔨-hall-of-shame'])
        channel: TextChannel = ctx.channel
        chl: TextChannel = await channel.clone(reason=reason)
        await chl.edit(position=channel.position)
        await channel.delete()
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{ctx.author.mention} has nuked **{channel.name}**.'
        log.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)
        nuked: Embed = Embed(title='Nuked', timestamp=datetime.utcnow(), color=0xff0000)
        nuked.set_image(url='https://media1.giphy.com/media/jmSImqrm28Vdm/200.gif')
        nuked.set_author(name=self.server_name(ctx.author), icon_url=ctx.author.avatar_url)
        nuked.description = f'This channel has been nuked by {ctx.author.mention}.'
        await chl.send(embed=nuked)

    @group(aliases=['auditlog'], invoke_without_command=True, case_insensitive=True)
    @has_permissions(administrator=True)
    async def audit(self, ctx: Context):
        if ctx.guild is None:
            return
        if ctx.invoked_subcommand is None:
            info: Embed = Embed(title='Audit Log commands:', color=Colour.random(), timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}audit ACTION|USER|TARGET <action|user> [limit]'
            info.set_footer(text=self.server_name(ctx.guild.me), icon_url=ctx.guild.me.avatar_url)
            await ctx.send(embed=info)

    @audit.command()
    async def action(self, ctx: Context, act: str = None, limit: int = None):
        if act is None:
            info: Embed = Embed(title='Audit Log commands:', color=Colour.random(), timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}audit ACTION action [limit]'
            info.set_footer(text=self.server_name(ctx.guild.me), icon_url=ctx.guild.me.avatar_url)
            await ctx.send(embed=info)
            return
        actions: dict[str, AuditLogAction] = {
            'guild_update': AuditLogAction.guild_update,
            'channel_create': AuditLogAction.channel_create,
            'channel_update': AuditLogAction.channel_delete,
            'channel_delete': AuditLogAction.channel_delete,
            'overwrite_create': AuditLogAction.overwrite_create,
            'overwrite_update': AuditLogAction.overwrite_update,
            'overwrite_delete': AuditLogAction.overwrite_delete,
            'kick': AuditLogAction.kick,
            'member_prune': AuditLogAction.member_prune,
            'ban': AuditLogAction.ban,
            'unban': AuditLogAction.unban,
            'member_update': AuditLogAction.member_update,
            'member_role_update': AuditLogAction.member_role_update,
            'member_move': AuditLogAction.member_move,
            'member_disconnect': AuditLogAction.member_disconnect,
            'bot_add': AuditLogAction.bot_add,
            'role_create': AuditLogAction.role_create,
            'role_update': AuditLogAction.role_update,
            'role_delete': AuditLogAction.role_delete,
            'invite_create': AuditLogAction.invite_create,
            'invite_update': AuditLogAction.invite_update,
            'invite_delete': AuditLogAction.invite_delete,
            'webhook_create': AuditLogAction.webhook_create,
            'webhook_update': AuditLogAction.webhook_update,
            'webhook_delete': AuditLogAction.webhook_delete,
            'emoji_create': AuditLogAction.emoji_create,
            'emoji_update': AuditLogAction.emoji_update,
            'emoji_delete': AuditLogAction.emoji_delete,
            'message_delete': AuditLogAction.message_delete,
            'message_bulk_delete': AuditLogAction.message_bulk_delete,
            'message_pin': AuditLogAction.message_pin,
            'message_unpin': AuditLogAction.message_unpin,
            'integration_create': AuditLogAction.integration_create,
            'integration_update': AuditLogAction.integration_update,
            'integration_delete': AuditLogAction.integration_delete
        }
        if actions[act] is not None:
            guild: Guild = ctx.guild
            logs: AuditLogIterator = guild.audit_logs(limit=limit, action=actions[act.lower()])
            entries: list[AuditLogEntry] = await logs.flatten()
            paginator: Paginator = Paginator(prefix='', suffix='')
            for entry in entries:
                paginator.add_line(
                    f'User: **{self.server_name(entry.user)}** Action: **{entry.action}** Target: '
                    f'**{self.server_name(entry.target) if entry.target is not None else "None"}** At: '
                    f'**{entry.created_at}**')
            for page in paginator.pages:
                await ctx.send(page)
        else:
            await ctx.send(f'Invalid action. Please use '
                           f'**{self.bot.command_prefix}audit actions** to see which actions are available.')

    @audit.command()
    async def actions(self, ctx: Context):
        actions: dict[str, AuditLogAction] = {
            'guild_update': AuditLogAction.guild_update,
            'channel_create': AuditLogAction.channel_create,
            'channel_update': AuditLogAction.channel_delete,
            'channel_delete': AuditLogAction.channel_delete,
            'overwrite_create': AuditLogAction.overwrite_create,
            'overwrite_update': AuditLogAction.overwrite_update,
            'overwrite_delete': AuditLogAction.overwrite_delete,
            'kick': AuditLogAction.kick,
            'member_prune': AuditLogAction.member_prune,
            'ban': AuditLogAction.ban,
            'unban': AuditLogAction.unban,
            'member_update': AuditLogAction.member_update,
            'member_role_update': AuditLogAction.member_role_update,
            'member_move': AuditLogAction.member_move,
            'member_disconnect': AuditLogAction.member_disconnect,
            'bot_add': AuditLogAction.bot_add,
            'role_create': AuditLogAction.role_create,
            'role_update': AuditLogAction.role_update,
            'role_delete': AuditLogAction.role_delete,
            'invite_create': AuditLogAction.invite_create,
            'invite_update': AuditLogAction.invite_update,
            'invite_delete': AuditLogAction.invite_delete,
            'webhook_create': AuditLogAction.webhook_create,
            'webhook_update': AuditLogAction.webhook_update,
            'webhook_delete': AuditLogAction.webhook_delete,
            'emoji_create': AuditLogAction.emoji_create,
            'emoji_update': AuditLogAction.emoji_update,
            'emoji_delete': AuditLogAction.emoji_delete,
            'message_delete': AuditLogAction.message_delete,
            'message_bulk_delete': AuditLogAction.message_bulk_delete,
            'message_pin': AuditLogAction.message_pin,
            'message_unpin': AuditLogAction.message_unpin,
            'integration_create': AuditLogAction.integration_create,
            'integration_update': AuditLogAction.integration_update,
            'integration_delete': AuditLogAction.integration_delete
        }
        paginator: Paginator = Paginator(prefix='', suffix='')
        for key in actions.keys():
            paginator.add_line(f'Action: **{key}**')
        for page in paginator.pages:
            await ctx.send(page)

    @audit.command()
    async def user(self, ctx: Context, user: Union[User, Member] = None, limit: int = None):
        if user is None:
            info: Embed = Embed(title='Audit Log commands:', color=Colour.random(), timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}audit USER user [limit]'
            info.set_footer(text=self.server_name(ctx.guild.me), icon_url=ctx.guild.me.avatar_url)
            await ctx.send(embed=info)
        else:
            guild: Guild = user.guild if isinstance(user, Member) else ctx.guild
            logs: AuditLogIterator = guild.audit_logs(limit=limit, user=user, oldest_first=True)
            entries: list[AuditLogEntry] = await logs.flatten()
            paginator: Paginator = Paginator(prefix='', suffix='')
            for entry in entries:
                paginator.add_line(
                    f'User: **{self.server_name(entry.user)}** Action: **{entry.action}** Target: '
                    f'**{self.server_name(entry.target) if entry.target is not None else "None"}** At: '
                    f'**{entry.created_at}**')
            for page in paginator.pages:
                await ctx.send(page)


def setup(bot: Bot):
    bot.add_cog(Moderation(bot))

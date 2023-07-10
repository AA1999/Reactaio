import re
from datetime import datetime
from enum import IntEnum
from typing import Optional, Union

from asyncpg import Connection
from discord import Embed, Message, Emoji, Role, Guild, TextChannel, HTTPException, RawReactionActionEvent, Member, \
    NotFound, PartialEmoji
from discord.ext.commands import Bot, Cog, group, Context, guild_only
from emojiconverter import EmojiConverter, EmojiNotFound


class Mode(IntEnum):
    GIVE = 1,
    TAKE = 2,
    GIVEONLY = 3,
    TAKEONLY = 4,
    GIVEANDTAKE = 5


class ReactionRoles(Cog):
    bot: Bot

    def __init__(self, bot: Bot):
        self.bot = bot

    def get_role(self, role_name: Union[Role, int, str]):
        if isinstance(role_name, Role):
            return role_name
        if isinstance(role_name, int):
            guild: Guild = self.bot.get_guild(812314425318440961)
            return guild.get_role(role_name)
        if isinstance(role_name, str):
            guild: Guild = self.bot.get_guild(812314425318440961)
            roles: list[Role] = guild.roles
            roles.reverse()
            for role in roles:
                if role_name.lower().startswith(role.name.lower()):
                    return role
                if role_name.lower() in role.name.lower():
                    return role
            return None

    @guild_only()
    @group(aliases=['rr', 'reactrole', 'rrole'], invoke_without_subcommand=True, case_insensitive=True)
    async def reactionrole(self, ctx: Context):
        if ctx.invoked_subcommand is None:
            info: Embed = Embed(title='ReactionRole', color=0x0000ff, timestamp=datetime.utcnow())
            info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
            info.description = f'{self.bot.command_prefix}reactionrole add/addmany/update/remove/unique/post [channel] ' \
                               f'[message] [type] [unique] [emoji] [role] [emoji2] [role2]... '
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}reactionrole add #cool-channel '
                                                   f'123456789101112 1 yes 😂 "cool role"\n')
            await ctx.send(embed=info)

    @reactionrole.command(aliases=['a'])
    async def add(self, ctx: Context, channel: Optional[TextChannel], messageid: Optional[int], type: Optional[int],
                  isunique: Optional[bool],
                  emote: Union[Emoji, PartialEmoji, str], role: Union[Role, int, str, None],
                  roletotake: Union[Role, int, str, None]):
        await self.bot.wait_until_ready()
        mode: Mode = Mode(type)
        if channel is None:
            info: Embed = Embed(title='ReactionRole', color=0x0000ff, timestamp=datetime.utcnow())
            info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
            info.description = f'{self.bot.command_prefix}reactionrole add [channel] [message] [type] [is unique] [' \
                               'emoji] [role] <role to take> '
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}reactionrole add #channel 123343 1 true '
                                                   f'📧 123456789101112')
            await ctx.send(embed=info)
            return
        try:
            ec: EmojiConverter = EmojiConverter()
            emoji: Emoji = await ec.convert(ctx, emote)
        except EmojiNotFound:
            await ctx.send(f"Invalid emoji '{emote}', please try again.")
            return
        message: Message = await channel.fetch_message(messageid)
        await message.add_reaction(emoji)
        if mode == Mode.GIVEANDTAKE:
            r = self.get_role(role)
            r2 = self.get_role(roletotake)
            if r is None or r2 is None:
                await ctx.send('One or two of the roles are not found, please try again.')
                return
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    await connection.execute('INSERT INTO reactionroles(messageid, isunique, type, emoji, role, '
                                             'roletotake) VALUES ($1, $2, $3, $4, $5, $6) ON CONFLICT (role) DO UPDATE '
                                             'SET isunique = $2, type = $3, emoji = $4, role = $5, roletotake = $6',
                                             message.id, isunique, type, str(emoji), r.id, r2.id, )
        else:
            r = self.get_role(role)
            if r is None:
                await ctx.send('One or two of the roles are not found, please try again.')
                return
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    await connection.execute('INSERT INTO reactionroles(messageid, isunique, type, emoji, role) '
                                             'VALUES ($1, $2, $3, $4, $5) ON CONFLICT (role) DO UPDATE '
                                             'SET isunique = $2, type = $3, emoji = $4, role = $5',
                                             message.id, isunique, type, str(emoji), r.id, )
        success: Embed = Embed(title='Success!', color=0x00ff00, timestamp=datetime.utcnow())
        success.description = f'The following reaction role has been made: {str(emoji)}: +{role.mention} ' \
                              f"{f'-{roletotake.mention}' if roletotake is not None else ''}"
        await ctx.send(embed=success)

    @reactionrole.command(aliases=['am', 'addm'])
    async def addmany(self, ctx: Context, channel: TextChannel, messageid: Optional[int], type: Optional[int],
                      unique: Optional[bool], *, args: Optional[str]):
        if channel is None:
            info: Embed = Embed(title='ReactionRole', color=0x00ffcc, timestamp=datetime.utcnow())
            info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
            info.description = f'{self.bot.command_prefix}reactionrole addmany [channel] [message] [type] [is unique]\n' \
                               '[emoji] [role] <role to take> \n' \
                               '[emoji] [role] <role to take> \n'
            info.add_field(name='Example: ',
                           value=f'{self.bot.command_prefix}reactionrole addmany #chl 123343 1 true \n'
                                 f'📧 123456789101112\n'
                                 f'🎟 Cool role\n')
            await ctx.send(embed=info)
            return
        mode = Mode(type)
        if mode == Mode.GIVEANDTAKE:
            lines = args.split('\n')
            for line in lines:
                line = re.sub(' {2,}', ' ', line)
                try:
                    ec: EmojiConverter = EmojiConverter()
                    emoji: Emoji = await ec.convert(ctx, line.split(' ', 2)[0])
                except EmojiNotFound:
                    await ctx.send(f"Invalid emoji{line.split(' ', 2)[0]}, please try again.")
                    return
                role = self.get_role(line.split(' ', 2)[1])
                roletotake = self.get_role(line.split(' ', 2)[2])
                if role is None or roletotake is None:
                    await ctx.send('One or more of the roles are not found, please try again')
                    return
                await self.add(ctx, channel, messageid, type, unique, emoji, role, roletotake)
        else:
            lines = args.split('\n')
            for line in lines:
                try:
                    ec: EmojiConverter = EmojiConverter()
                    emoji: Emoji = await ec.convert(ctx, line.split(' ', 1)[0])
                except EmojiNotFound:
                    await ctx.send(f"Invalid emoji{line.split(' ', 1)[0]}, please try again.")
                    return
                role = self.get_role(line.split(' ', 1)[1])
                if role is None:
                    await ctx.send('One or more of the roles are not found, please try again')
                    return
                await self.add(ctx, channel, messageid, type, unique, emoji, role, None)

    @reactionrole.command(aliases=['u'])
    async def unique(self, ctx: Context, messageid: Optional[int]):
        if messageid is None:
            info: Embed = Embed(title='ReactionRole', color=0x00ffcc, timestamp=datetime.utcnow())
            info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
            info.description = f'{self.bot.command_prefix}reactionrole unique [message]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}reactionrole unique 123343')
            await ctx.send(embed=info)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    await connection.execute('UPDATE reactionroles SET isunique = True WHERE messageid = $1',
                                             messageid, )
                    await ctx.send('Message successfully marked as unique.')
                except:
                    await ctx.send('Message with that id does not have a reaction role/does not exists.')

    @reactionrole.command(aliases=['n', 'norm'])
    async def normal(self, ctx: Context, messageid: Optional[int]):
        if messageid is None:
            info: Embed = Embed(title='ReactionRole', color=0x00ffcc, timestamp=datetime.utcnow())
            info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
            info.description = f'{self.bot.command_prefix}reactionrole normal [message]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}reactionrole normal 123343')
            await ctx.send(embed=info)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    await connection.execute('UPDATE reactionroles SET isunique = FALSE WHERE messageid = $1',
                                             messageid, )
                    await ctx.send('Message successfully marked as normal.')
                except:
                    await ctx.send('Message with that id does not have a reaction role/does not exists.')

    @reactionrole.command(aliases=['edit', 'e', 'up'])
    async def update(self, ctx: Context, messageid: Optional[int], emoji: Optional[Emoji], *, role: Union[Role, str,
                                                                                                          None]):
        if messageid is None:
            info: Embed = Embed(title='ReactionRole', color=0x00ffcc, timestamp=datetime.utcnow())
            info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
            info.description = f'{self.bot.command_prefix}reactionrole update [message] [emoji] [role]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}reactionrole normal 123343 📧 cool role')
            await ctx.send(embed=info)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    await connection.execute('UPDATE reactionroles SET role = $1 WHERE emoji = $2 AND messageid = $3',
                                             self.get_role(role).id, str(emoji), messageid)
                    await ctx.send(f'Successfully updated the emoji {str(emoji)} with the role '
                                   f'`{self.get_role(role).mention}`')
                except:
                    await ctx.send('Invalid emoji. Try again.')

    @reactionrole.command(aliases=['r', 'rem', 'd', 'del', 'delete'])
    async def remove(self, ctx: Context, channel: Optional[TextChannel], messageid: Optional[int]):
        if channel is None:
            info: Embed = Embed(title='ReactionRole', color=0x00ffcc, timestamp=datetime.utcnow())
            info.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
            info.description = f'{self.bot.command_prefix}reactionrole remove [channel] [message]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}reactionrole remove #channel 123343')
            await ctx.send(embed=info)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    await connection.execute('DELETE FROM reactionroles WHERE messageid = $1', messageid, )
                    message: Message = await channel.fetch_message(messageid)
                    await message.clear_reactions()
                    await ctx.send(f'Successfully removed all reaction roles for message {messageid}.')
                except NotFound:
                    await ctx.send('Invalid message id. Please try again.')
                except:
                    await ctx.send(f'Message {messageid} does not contain reaction roles.')

    @reactionrole.command(aliases=['c', 'clean'])
    async def clear(self, ctx: Context):
        await ctx.send('This will delete all reaction roles in the server (not the reactions themselves), '
                       'are you sure? (yes/y/no/n)')

        def check(message: Message):
            return message.content.lower() in ['yes', 'y', 'no', 'n'] and message.author == ctx.author

        message: Message = await self.bot.wait_for('message', check=check, timeout=None)
        if message.content.lower() in ['no', 'n']:
            return
        try:
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    messages = await connection.fetch('DELETE FROM reactionroles RETURNING messageid')
            await ctx.send('Successfully cleared all reactions.')
        except HTTPException:
            pass
        finally:
            await ctx.send('There were errors processing the request.')

    @Cog.listener()
    async def on_raw_reaction_add(self, payload: RawReactionActionEvent):
        guild: Guild = self.bot.get_guild(payload.guild_id)
        member: Member = payload.member
        if member.bot:
            return
        channel: TextChannel = guild.get_channel(payload.channel_id)
        message: Message = await channel.fetch_message(payload.message_id)
        emoji: Emoji = payload.emoji
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                ops = await connection.fetchrow('SELECT * FROM reactionroles WHERE messageid = $1 AND emoji = $2',
                                                message.id, str(emoji), )
        if ops is None:
            return
        mode: Mode = Mode(ops['type'])
        if mode == Mode.GIVEANDTAKE:
            role: Role = guild.get_role(ops['role'])
            roletotake: Role = guild.get_role(ops['roletotake'])
            roles: set[Role] = set(member.roles)
            roles.add(role)
            roles.remove(roletotake)
            if ops['isunique']:
                connection: Connection
                async with self.bot.pool.acquire() as connection:
                    async with connection.transaction():
                        rs = await connection.fetch('SELECT role, roletotake, emoji FROM reactionroles WHERE '
                                                    'messageid = $1 AND emoji <> $2', message.id, str(emoji))
            ec: EmojiConverter = EmojiConverter()
            for r in rs:
                try:
                    roles.remove(r['role'])
                    roles.add(r['roletotake'])
                    await message.remove_reaction(await ec.convert(await self.bot.get_context(message), r['emoji']), member)
                except KeyError:
                    pass
                except NotFound:
                    pass
            await member.edit(roles=list(roles), reason='Reaction role react.')

        elif mode in [Mode.GIVE, Mode.GIVEONLY]:
            role: Role = guild.get_role(ops['role'])
            roles: set[Role] = set(member.roles)
            roles.add(role)
            if ops['isunique']:
                connection: Connection
                async with self.bot.pool.acquire() as connection:
                    async with connection.transaction():
                        rs = await connection.fetch('SELECT role, emoji FROM reactionroles WHERE messageid = $1 AND '
                                                    'emoji <> $2', message.id, str(emoji))
                ec: EmojiConverter = EmojiConverter()
                for r in rs:
                    try:
                        roles.remove(r['role'])
                        await message.remove_reaction(await ec.convert(await self.bot.get_context(message), r['emoji']),
                                                      member)
                    except KeyError:
                        pass
                    except NotFound:
                        pass
                await member.edit(roles=list(roles), reason='Reaction role react.')
        elif mode in [Mode.TAKE, Mode.TAKEONLY]:
            role: Role = guild.get_role(ops['role'])
            await member.remove_roles(role, reason='Reaction role react.')

    @Cog.listener()
    async def on_raw_reaction_remove(self, payload: RawReactionActionEvent):
        guild: Guild = self.bot.get_guild(payload.guild_id)
        member: Member = guild.get_member(payload.user_id)
        if member.bot:
            return
        channel: TextChannel = guild.get_channel(payload.channel_id)
        message: Message = await channel.fetch_message(payload.message_id)
        emoji: Emoji = payload.emoji
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                try:
                    ops = await connection.fetchrow('SELECT * FROM reactionroles WHERE messageid = $1 AND emoji = $2',
                                                    message.id, str(emoji))
                except:
                    pass
        if ops is None:
            return
        mode: Mode = Mode(ops['type'])
        if mode == Mode.GIVEANDTAKE:
            role: Role = guild.get_role(ops['role'])
            roletotake: Role = guild.get_role(ops['roletotake'])
            roles: set[Role] = set(member.roles)
            try:
                roles.remove(role)
                roles.add(roletotake)
                await member.edit(roles=list(roles), reason='Reaction roles de-react.')
            except NotFound:
                pass

        elif mode == Mode.TAKE:
            role: Role = guild.get_role(ops['role'])
            roles: set[Role] = set(member.roles)
            roles.add(role)
            if ops['isunique']:
                connection: Connection
                async with self.bot.pool.acquire() as connection:
                    async with connection.transaction():
                        rs = await connection.fetch('SELECT role, emoji FROM reactionroles WHERE messageid = $1 AND '
                                                    'emoji <> $2', message.id, str(emoji))
                ec: EmojiConverter = EmojiConverter()
                for r in rs:
                    try:
                        roles.remove(r['role'])
                        await message.remove_reaction(await ec.convert(await self.bot.get_context(message), r['emoji']),
                                                      member)
                    except NotFound:
                        pass
                    except HTTPException:
                        pass
            await member.edit(roles=list(roles), reason='Reaction roles de-react.')

        elif mode == Mode.GIVE:
            role: Role = guild.get_role(ops['role'])
            await member.remove_roles(role, reason='Reaction roles de-react.')


def setup(bot: Bot):
    bot.add_cog(ReactionRoles(bot))

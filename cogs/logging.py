from datetime import datetime
from typing import Optional, Sequence, Union

from discord import AuditLogEntry, Message, Role, User, Emoji, Invite
from discord.channel import TextChannel
from discord.embeds import Embed
from discord.enums import AuditLogAction
from discord.ext.commands import Bot, Cog
from discord.guild import Guild
from discord.iterators import AuditLogIterator
from discord.member import Member, VoiceState


class Logging(Cog):
    bot: Bot
    id: int

    def __init__(self, bot: Bot) -> None:
        self.bot = bot
        with open('log.data', mode='r') as f:
            line = f.readline()
            self.id = int(line) + 1

    def server_name(self, member: Union[Member, User]):
        return f'{member.display_name}#{member.discriminator}'

    def __humans__(self):
        guild: Guild = self.bot.get_guild(812314425318440961)
        member: Member
        return sum(not member.bot for member in guild.members)

    @Cog.listener()
    async def on_member_join(self, member: Member):
        await self.bot.wait_until_ready()
        joinlogs: TextChannel = member.guild.get_channel(self.bot.channels['join-leave-logs'])
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{member.mention} joined the server. We\'re now at {self.__humans__()} members.'
        log.set_footer(text=member.guild.name, icon_url=member.guild.icon_url)
        log.set_author(name=self.server_name(member), icon_url=member.avatar_url)
        await joinlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_member_remove(self, member: Member):
        await self.bot.wait_until_ready()
        guild: Guild = member.guild
        joinlogs: TextChannel = member.guild.get_channel(self.bot.channels['join-leave-logs'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=True, action=AuditLogAction.kick)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        kicked: bool = entry.target.id == member.id
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{member.mention} {"left the server" if not kicked else f"was kicked by {entry.user.mention}"}. We\'re now at {self.__humans__()} members. '
        log.set_footer(text=member.guild.name, icon_url=member.guild.icon_url)
        log.set_author(name=self.server_name(member), icon_url=member.avatar_url)
        await joinlogs.send(embed=log)
        if kicked and not entry.user.bot:
            modlog: TextChannel = guild.get_channel(self.bot.channels['moderation-log'])
            log2: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
            log2.description = f'{member.mention} was kicked by {entry.user.mention}. Reason: {entry.reason}.'
            log2.set_footer(text=member.guild.name, icon_url=member.guild.icon_url)
            log2.set_author(name=self.server_name(member), icon_url=member.avatar_url)
            hallofshame: TextChannel = self.bot.get_channel(self.bot.channels['🔨-hall-of-shame'])
            await hallofshame.send(embed=log)
            await modlog.send(embed=log2)
        roles: list[str] = []
        role: Role
        for role in member.roles:
            if role == member.guild.default_role:
                continue
            roles.append(f'**{role.name}**, \n')
        roles.reverse()
        rolestr: str = ''.join(roles)
        rolestr = rolestr[:-3]
        await joinlogs.send(f'{member}\'s Roles:\n {rolestr}')
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_message_delete(self, message: Message):
        await self.bot.wait_until_ready()
        guild: Guild = message.guild
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.message_delete)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        messagelogs: TextChannel = guild.get_channel(self.bot.channels['message-log'])
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has deleted message {message.id}: \n'
        log.add_field(name=f'{message.id}\'s content: ', value=message.content if message.content is not None and
                                                                                  message.content != '' else
        message.embeds[0].description, inline=False)
        log.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await messagelogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_message_edit(self, before: Message, after: Message):
        await self.bot.wait_until_ready()
        guild: Guild = before.guild
        messagelogs: TextChannel = guild.get_channel(self.bot.channels['message-log'])
        logb: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        logb.description = f'{self.server_name(before.author)} has edited message {before.id} at ' \
                           f'{after.edited_at}: '
        logb2: Embed = Embed(title=f'From: ', color=0x00ff00, timestamp=datetime.utcnow())
        logb2.description = before.content if before.content is not None else before.embeds[0]
        logb.set_author(name=self.server_name(before.author), icon_url=before.author.avatar_url)
        logb.set_footer(text=self.server_name(before.author), icon_url=before.guild.icon_url)
        logb2.set_author(name=self.server_name(before.author), icon_url=before.author.avatar_url)
        logb2.set_footer(text=self.server_name(before.author), icon_url=before.guild.icon_url)
        loga: Embed = Embed(title='To: ', color=0x00ff00, timestamp=datetime.utcnow())
        loga.description = f'{after.content if after.content is not None else after.embeds[0]}\n\n[Jump to Message]' \
                           f'({after.jump_url})'
        loga.set_author(name=self.server_name(before.author), icon_url=before.author.avatar_url)
        loga.set_footer(text=guild.name, icon_url=before.guild.icon_url)
        await messagelogs.send(embed=logb)
        await messagelogs.send(embed=logb2)
        await messagelogs.send(embed=loga)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_channel_create(self, channel: TextChannel):
        await self.bot.wait_until_ready()
        guild: Guild = channel.guild
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.channel_create)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has created channel {channel.mention} ({channel.id}).'
        log.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_channel_delete(self, channel: TextChannel):
        await self.bot.wait_until_ready()
        guild: Guild = channel.guild
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.channel_delete)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has deleted channel **{channel.name}** ({channel.id}).'
        log.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_channel_update(self, before: TextChannel, after: TextChannel):
        await self.bot.wait_until_ready()
        guild: Guild = before.guild
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.channel_delete)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has edited channel {before.mention} ({before.id}).'
        log.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        change: str
        if before.name != after.name:
            change = f'Channel name from **{before.name}** to **{after.name}**.'
        elif isinstance(before, TextChannel) and before.topic != after.topic:
            change = f'Channel topic from **{before.topic}** to **{after.topic}**.'
        elif before.overwrites != after.overwrites:
            change = f'Channel overwrite from **{before.overwrites}** to **{after.overwrites}**'
            await serverlogs.send(embed=Embed(title='Overwrites', color=0x00ff00, description=change))
            await serverlogs.send(embed=log)
            return
        else:
            change = f'Channel change undefined.'
        log.add_field(name='Change: ', value=change)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_channel_pins_update(self, channel: TextChannel, last_pin: Optional[datetime]):
        await self.bot.wait_until_ready()
        guild: Guild = channel.guild
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.message_pin)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        pins: list[Message] = await channel.pins()
        pinned: Message = pins[-1]
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has pinned message {pinned.id}): '
        log.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        msg: Embed = Embed(title=f'Message {pinned.id}\'s content: ', color=0x00ff00, timestamp=datetime.utcnow())
        msg.description = f'{pinned.content}\n[Jump to Message]({pinned.jump_url} '
        msg.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        msg.set_footer(text=guild.name, icon_url=guild.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_member_update(self, before: Member, after: Member):
        await self.bot.wait_until_ready()
        guild: Guild = before.guild
        memberlogs: TextChannel = guild.get_channel(self.bot.channels['member-logs'])
        change: str
        if before.nick != after.nick:
            logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                      action=AuditLogAction.member_update)
            entries: list[AuditLogEntry] = await logs.flatten()
            entry: AuditLogEntry = entries[0]
            change = f'{entry.user.mention} has changed names from **{self.server_name(before)} ' \
                     f'to **{self.server_name(after)}**.'
        elif before.roles != after.roles:
            logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                      action=AuditLogAction.member_role_update)
            entries: list[AuditLogEntry] = await logs.flatten()
            entry: AuditLogEntry = entries[0]
            broles: list[str] = []
            aroles: list[str] = []
            for role in before.roles:
                if role == guild.default_role:
                    continue
                broles.append(f'{role.mention}, \n')
            broles.reverse()
            for role in after.roles:
                if role == guild.default_role:
                    continue
                aroles.append(f'{role.mention}, \n')
            aroles.reverse()
            aroles: set[str] = set(aroles)
            broles: set[str] = set(broles)
            added = ''.join(aroles.difference(broles))
            added = added[:-3]
            removed = ''.join(broles.difference(aroles))
            removed = removed[:-3]
            change = f'{entry.user.mention} has updated roles for {entry.target.mention}. {f"Added {added}. " if len(added) != 0 else ""}' \
                     f'{f"Removed {removed}." if len(removed) != 0 else ""}'
        else:
            return
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = change
        log.set_author(name=entry.user, icon_url=entry.user.avatar_url)
        log.set_footer(text=before.guild.name, icon_url=before.guild.icon_url)
        await memberlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_user_update(self, before: User, after: User):
        await self.bot.wait_until_ready()
        guild: Guild = self.bot.get_guild(812314425318440961)
        memberlogs: TextChannel = guild.get_channel(self.bot.channels['member-logs'])
        if after.name != before.name:
            change = f'User {before.mention} has changed their name from **{before.name}** to **{after.name}**.'
        elif before.discriminator != after.discriminator:
            change = f'User {before.mention} changed their username from **{before.name}** to **{after.name}**.'
        else:
            change = f'User {before.mention} has updated their avatar.'
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = change
        log.set_author(name=f'{self.server_name(before)}', icon_url=after.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        log.set_thumbnail(url=before.avatar_url)
        log.set_image(url=after.avatar_url)
        await memberlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_update(self, before: Guild, after: Guild):
        await self.bot.wait_until_ready()
        serverlogs: TextChannel = before.get_channel(self.bot.channels['server-log'])
        logs: AuditLogIterator = after.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.guild_update)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        change: str = ''
        if before.name != after.name:
            change = f'{entry.user.mention} has updated **{before.name}**\'s name to **{after.name}**.'
        elif before.afk_channel != after.afk_channel:
            change = f'{entry.user.mention} has updated **{before.name}**\'s AFK channel from ' \
                     f'{before.afk_channel.mention} to {after.afk_channel.mention}.'
        elif before.afk_timeout != after.afk_timeout:
            change = f'{entry.user.mention} has changed guild afk timeout from {before.afk_timeout} minutes to ' \
                     f'{after.afk_timeout}. '
        else:
            change = 'Unidentified change.'
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = change
        log.set_author(name=entry.user, icon_url=entry.user.avatar_url)
        log.set_footer(text=after.name, icon_url=after.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_role_create(self, role: Role):
        await self.bot.wait_until_ready()
        guild: Guild = role.guild
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.role_create)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has created role {role.mention} ({role.id}) in guild **{role.guild}**.'
        log.set_author(name=entry.user, icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_role_delete(self, role: Role):
        await self.bot.wait_until_ready()
        guild: Guild = role.guild
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.role_delete)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has delete role **{role.name}** in guild **{role.guild}**.'
        log.set_author(name=entry.user, icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_role_update(self, before: Role, after: Role):
        await self.bot.wait_until_ready()
        guild: Guild = before.guild
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.role_update)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        change: str = ''
        if before.name != after.name:
            change = f'{entry.user.mention} has updated **{before.name}**\'s name to {after.mention}.'
        elif before.color != after.color:
            change = f'{entry.user.mention} has updated **{before.name}**\'s color from **{before.color}** to **{after.color}**.'
        elif before.position != after.position:
            change = f'{entry.user.mention} has updated **{after.name}**\'s position.'
        else:
            change = 'Unidentified change.'
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = change
        log.set_author(name=entry.user, icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_guild_emojis_update(self, guild: Guild, before: Sequence[Emoji], after: Sequence[Emoji]):
        await self.bot.wait_until_ready()
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False,
                                                  action=AuditLogAction.emoji_update)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has updated **{guild.name}**\'s emojis.'
        log.add_field(name='Old Emojis: ', value=str(before))
        log.add_field(name='New Emojis: ', value=str(after))
        log.set_author(name=entry.user, icon_url=entry.user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_voice_state_update(self, member: Member, before: VoiceState, after: VoiceState):
        await self.bot.wait_until_ready()
        guild: Guild = member.guild
        voicelogs: TextChannel = guild.get_channel(self.bot.channels['voice-logs'])
        if before.channel is None:
            change = f'{member} has joined voice channel **{after.channel.name}**'
        else:
            change = f'{member} has left voice channel **{before.channel.name}**'
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = change
        log.set_author(name=f'{self.server_name(member)}', icon_url=member.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await voicelogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_member_ban(self, guild: Guild, user: Union[User, Member]):
        await self.bot.wait_until_ready()
        modlog: TextChannel = guild.get_channel(self.bot.channels['moderation-log'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False, action=AuditLogAction.ban)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        if entry.user.bot:
            return
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has banned {user.mention} ({user.id}). Reason: {entry.reason}'
        log.set_author(name=f'{self.server_name(entry.user)}', icon_url=user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        hallofshame: TextChannel = self.bot.get_channel(self.bot.channels['🔨-hall-of-shame'])
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_member_unban(self, guild: Guild, user: User):
        await self.bot.wait_until_ready()
        modlog: TextChannel = guild.get_channel(self.bot.channels['moderation-log'])
        hallofshame: TextChannel = self.bot.get_channel(self.bot.channels['🔨-hall-of-shame'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False, action=AuditLogAction.unban)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        if entry.user.bot:
            return
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has unbanned {user.mention} ({user.id}). Reason: {entry.reason}'
        log.set_author(name=f'{self.server_name(entry.user)}', icon_url=user.avatar_url)
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        await modlog.send(embed=log)
        await hallofshame.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_invite_create(self, invite: Invite):
        await self.bot.wait_until_ready()
        guild: Guild = invite.guild
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False, action=AuditLogAction.invite_create)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has created{" temporary" if invite.temporary else ""} invite code ' \
                          f'**{invite.code}** at {invite.created_at}.'
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        log.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1

    @Cog.listener()
    async def on_invite_delete(self, invite: Invite):
        await self.bot.wait_until_ready()
        guild: Guild = invite.guild
        serverlogs: TextChannel = guild.get_channel(self.bot.channels['server-log'])
        logs: AuditLogIterator = guild.audit_logs(limit=1, oldest_first=False, action=AuditLogAction.invite_create)
        entries: list[AuditLogEntry] = await logs.flatten()
        entry: AuditLogEntry = entries[0]
        log: Embed = Embed(title=f'Log Entry #{self.id}: ', color=0x00ff00, timestamp=datetime.utcnow())
        log.description = f'{entry.user.mention} has deleted{" temporary" if invite.temporary else ""} invite code ' \
                          f'**{invite.code}** at {invite.created_at}.'
        log.set_footer(text=guild.name, icon_url=guild.icon_url)
        log.set_author(name=self.server_name(entry.user), icon_url=entry.user.avatar_url)
        await serverlogs.send(embed=log)
        with open('log.data', mode='w') as f:
            f.write(str(self.id))
        self.id += 1


def setup(bot: Bot):
    bot.add_cog(Logging(bot))

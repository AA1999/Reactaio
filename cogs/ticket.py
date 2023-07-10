from asyncio.locks import Lock
from asyncio.tasks import sleep
from datetime import datetime
from io import StringIO
from os import remove
from re import L
from typing import Union

from asyncpg.connection import Connection
from discord import Game, Status, NotFound
from discord.abc import User
from discord.channel import CategoryChannel, TextChannel
from discord.colour import Colour
from discord.embeds import Embed
from discord.ext.commands import Bot, Cog, Context
from discord.ext.commands.cooldowns import BucketType, CooldownMapping
from discord.ext.commands.core import command, has_any_role
from discord.file import File
from discord.guild import Guild
from discord.iterators import HistoryIterator
from discord.member import Member
from discord.message import Message, Attachment
from discord.permissions import PermissionOverwrite
from discord.raw_models import RawReactionActionEvent
from discord.reaction import Reaction
from discord.role import Role
from jinja2 import Template


class Ticket(Cog):
    bot: Bot
    lock: bool
    _lock: Lock
    generalticket: CategoryChannel
    applyticket: CategoryChannel
    verificationticket: CategoryChannel
    sverify: Message
    vverify: Message
    rep: Message
    gen: Message
    app: Message
    apl: Message

    def isntpinned(self, message: Message):
        return message not in self.vpins and message not in self.mpins

    async def config(self):
        await self.bot.wait_until_ready()
        guild: Guild = self.bot.get_guild(812314425318440961)
        await self.bot.change_presence(status=Status.online, activity=Game(f'{self.bot.command_prefix}help'
                                                                           f' on {guild.name}.'))
        self.generalticket = self.bot.get_channel(824735392267894824)
        self.applyticket = self.bot.get_channel(825677447202603068)
        self.verificationticket = self.bot.get_channel(824734974191075358)

        makeaticket: TextChannel = self.bot.get_channel(self.bot.channels['make-a-ticket'])
        verification: TextChannel = self.bot.get_channel(self.bot.channels['verification'])

        self.vpins: list[Message] = await makeaticket.pins()
        self.mpins: list[Message] = await verification.pins()

        await makeaticket.purge(limit=100, check=self.isntpinned)
        await verification.purge(limit=100, check=self.isntpinned)

        selfie: Embed = Embed(title='Selfie Verification', color=0x000000,
                              description='Please react to this message with the specified emoji to make a ticket.')
        selfie.set_footer(text=guild.name, icon_url=guild.icon_url)
        self.sverify = await verification.send(embed=selfie)
        video: Embed = Embed(title='Video Verification', color=0x000000,
                             description='Please react to this message with the specified emoji to make a ticket.')
        video.set_footer(text=guild.name, icon_url=guild.icon_url)
        self.vverify = await verification.send(embed=video)
        report: Embed = Embed(title='Report a User', color=0x000000,
                              description='Please react to this message with the specified emoji to make a ticket.')
        report.set_footer(text=guild.name, icon_url=guild.icon_url)
        self.rep = await makeaticket.send(embed=report)
        general: Embed = Embed(title='General support', color=0x000000,
                               description='Please react to this message with the specified emoji to make a ticket.')
        general.set_footer(text=guild.name, icon_url=guild.icon_url)
        self.gen = await makeaticket.send(embed=general)
        apply: Embed = Embed(title='Mod/Bot Developer apply', color=0x000000,
                             description='Please react to this message with the specified emoji to make a ticket.')
        apply.set_footer(text=guild.name, icon_url=guild.icon_url)
        self.app = await makeaticket.send(embed=apply)
        appeal: Embed = Embed(title='Appeal punishment/ban', color=0x000000,
                              description='Please react to this message with the specified emoji to make a ticket.')
        appeal.set_footer(text=guild.name, icon_url=guild.icon_url)
        self.apl = await makeaticket.send(embed=appeal)

        await self.sverify.add_reaction('🎟')
        await self.vverify.add_reaction('🎟')
        await self.rep.add_reaction('🎟')
        await self.gen.add_reaction('🎟')
        await self.app.add_reaction('🎟')
        await self.apl.add_reaction('🎟')
        self.lock = False

    def __init__(self, bot: Bot) -> None:
        self.bot = bot
        self.lock = True
        self._lock = Lock()
        self._cd = CooldownMapping.from_cooldown(1, 10, BucketType.user)
        self.bot.loop.create_task(self.config())

    async def create_ticket(self, category: CategoryChannel, member: Member):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id: int = await connection.fetchval('SELECT id FROM tickets ORDER BY id DESC LIMIT 1')
                if id is None:
                    id = 1
                else:
                    id += 1
                name = f'ticket-{id: 05d}-{member.display_name}-{category.name}'
                guild: Guild = self.bot.get_guild(812314425318440961)
                headmod: Role = guild.get_role(813511889476386826)
                mod: Role = guild.get_role(812315517511008307)
                trialmod: Role = guild.get_role(812318396849848341)
                if category == self.generalticket:
                    overwrite = {
                        member: PermissionOverwrite(
                            read_messages=True,
                            view_channel=True,
                            send_messages=True,
                            embed_links=True,
                            attach_files=True
                        ),
                        guild.default_role: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        headmod: PermissionOverwrite(
                            read_messages=True,
                            view_channel=True,
                            send_messages=True,
                            embed_links=True,
                            attach_files=True
                        ),
                        mod: PermissionOverwrite(
                            read_messages=True,
                            view_channel=True,
                            send_messages=True,
                            embed_links=True,
                            attach_files=True
                        ),
                        trialmod: PermissionOverwrite(
                            read_messages=True,
                            view_channel=True,
                            send_messages=True,
                            embed_links=True,
                            attach_files=True
                        )
                    }
                elif category in [self.verificationticket, self.applyticket]:

                    overwrite = {

                        member: PermissionOverwrite(
                            read_messages=True,
                            view_channel=True,
                            send_messages=True,
                            embed_links=True,
                            attach_files=True
                        ),
                        guild.default_role: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        headmod: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        mod: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        trialmod: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        )
                    }

                ticket: TextChannel = await guild.create_text_channel(name=name, overwrites=overwrite,
                                                                      category=category,
                                                                      reason=f'Ticket by {member.display_name}'
                                                                             f'#{member.discriminator}')
                greet: Embed = Embed(ticket=category.name, color=Colour.random(), timestamp=datetime.utcnow())
                staff: Role = guild.get_role(814388827396243516)
                greet.description = 'Please wait until one of the staff members replies to the ticket.'
                greet.set_author(name=str(member), icon_url=member.avatar_url)
                message: Message = await ticket.send(content=f'Hey, {member.mention} ({staff.mention})', embed=greet)
                await message.add_reaction('🔒')
                await connection.execute(
                    'INSERT INTO tickets (name, channelid, datetimecreated, isclosed, authorid) VALUES ($1, $2, $3, '
                    '$4, $5)',
                    name, ticket.id, datetime.utcnow(), False, member.id, )

    async def close_ticket(self, channel: TextChannel, member: Member):
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                status = await connection.fetchrow('SELECT isclosed, authorid FROM tickets WHERE channelid = $1', channel.id, )
        if status['isclosed'] is None or status['authorid'] is None and not channel.name.startswith('ticket-'):
            await channel.send('This command is only usable in a ticket.')
            return
        elif status['isclosed'] == True:
            await channel.send('This ticket is already closed.')
            return
        guild: Guild = self.bot.get_guild(812314425318440961)
        user: Member = guild.get_member(status['authorid'])
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.execute('UPDATE tickets SET isclosed = $1 WHERE channelid = $2', True, channel.id)
        overwrite = {
            guild.default_role: PermissionOverwrite(
                read_messages=False,
                view_channel=False,
                send_messages=False,
                embed_links=False,
                attach_files=False
            ),

            member: PermissionOverwrite(
                read_messages=False,
                view_channel=False,
                send_messages=False,
                embed_links=False,
                attach_files=False
            ),
            user: PermissionOverwrite(

                read_messages=True,
                view_channel=True,
                send_messages=False,
                embed_links=False,
                attach_files=False
            ),

        }
        await channel.edit(overwrites=overwrite)
        embed: Embed = Embed(title='Closed', color=0xff0000, timestamp=datetime.utcnow(),
                                description=f'Ticket closed by {member.mention}.')
        message: Message = await channel.send(embed=embed)
        await message.add_reaction('🔓')
        await message.add_reaction('🔥')
        await message.add_reaction('📜')
        return message

    async def reopen_ticket(self, channel: TextChannel, member: Member):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                status = await connection.fetchrow('SELECT isclosed, authorid FROM tickets WHERE channelid = $1', channel.id, )

        if status['isclosed'] is None or status['authorid'] is None and not channel.name.startswith('ticket-'):
            await channel.send('This command is only usable in a ticket.')
            return
        elif status['isclosed'] == False:
            await channel.send('This ticket is not closed.')
            return
        guild: Guild = self.bot.get_guild(812314425318440961)
        headmod: Role = guild.get_role(813511889476386826)
        mod: Role = guild.get_role(812315517511008307)
        trialmod: Role = guild.get_role(812318396849848341)
        user: Member = guild.get_member(status['authorid'])
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.execute('UPDATE tickets SET isclosed = $1 WHERE channelid = $2', False, channel.id)
        if channel.category in [self.verificationticket, self.applyticket]:
            overwrite = {

                guild.default_role: PermissionOverwrite(
                    read_messages=False,
                    view_channel=False,
                    send_messages=False,
                    embed_links=False,
                    attach_files=False
                ),
                headmod: PermissionOverwrite(
                    read_messages=False,
                    view_channel=False,
                    send_messages=False,
                    embed_links=False,
                    attach_files=False
                ),
                mod: PermissionOverwrite(
                    read_messages=False,
                    view_channel=False,
                    send_messages=False,
                    embed_links=False,
                    attach_files=False
                ),
                trialmod: PermissionOverwrite(
                    read_messages=False,
                    view_channel=False,
                    send_messages=False,
                    embed_links=False,
                    attach_files=False
                ),
                member: PermissionOverwrite(
                    read_messages=True,
                    view_channel=True,
                    send_messages=True,
                    embed_links=True,
                    attach_files=True
                ),
                user: PermissionOverwrite(
                    read_messages=True,
                    view_channel=True,
                    send_messages=True,
                    embed_links=True,
                    attach_files=True
                )
            }
        elif channel.category == self.generalticket:
            overwrite = {
                guild.default_role: PermissionOverwrite(
                    read_messages=False,
                    view_channel=False,
                    send_messages=False,
                    embed_links=False,
                    attach_files=False
                ),
                headmod: PermissionOverwrite(
                    read_messages=True,
                    view_channel=True,
                    send_messages=True,
                    embed_links=True,
                    attach_files=True
                ),
                mod: PermissionOverwrite(
                    read_messages=True,
                    view_channel=True,
                    send_messages=True,
                    embed_links=True,
                    attach_files=True
                ),
                trialmod: PermissionOverwrite(
                    read_messages=True,
                    view_channel=True,
                    send_messages=True,
                    embed_links=True,
                    attach_files=True
                ),
                member: PermissionOverwrite(
                    read_messages=True,
                    view_channel=True,
                    send_messages=True,
                    embed_links=True,
                    attach_files=True
                ),
                user: PermissionOverwrite(
                    read_messages=True,
                    view_channel=True,
                    send_messages=True,
                    embed_links=True,
                    attach_files=True
                )
            }
        await channel.edit(overwrites=overwrite)
        embed: Embed = Embed(title='Re-opened', color=0xff0000, timestamp=datetime.utcnow(),
                                description=f'Ticket re-opened by {member.mention}.')
        message: Message = await channel.send(embed=embed)
        await message.add_reaction('🔒')
        return message

    async def delete_ticket(self, channel: TextChannel):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id = await connection.fetchval('SELECT authorid FROM tickets WHERE channelid = $1', channel.id, )
                if id is None and not channel.name.startswith('ticket-'):
                    await channel.send('This command is only usable in a ticket.')
                else:
                    await channel.delete()
                    try:
                        await connection.execute('DELETE FROM tickets WHERE channelid = $1', channel.id, )
                    except:
                        pass

    async def ticket_transcript(self, channel: TextChannel, member: Member):
        head: Template = Template('''

<!DOCTYPE html>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>{{user}}'s Ticket</title>
    <style>
        .authorimage {
            margin-right: 20px;
            width: 60px;
            height: 60px;
            border-radius: 50%;
        }
        
        .authorname {
            font-weight: bold
        }
        
        body {
            background-color: rgb(54, 57, 63);
            font-family: monospace;
            color: rgb(200, 208, 203);
        }
        
        .footerimg {
            border-radius: 10%;
            width: 20px;
            height: 20px;
            left: 10px;
            bottom: 1px;
        }
        
        .footer {
            font-size: 10px;
            bottom: 1px;
        }
        
        .container {
            padding: 10px;
            margin: 10px 0;
            width: 600px;
        }
        
        .container::after {
            content: "";
            clear: both;
            display: table;
        }
        
        .container img {
            float: left;
            margin-left: 20px;
        }
        
        .embed {
            background-color: #202027;
            border-radius: 3%;
            border-left-style: solid;
            border-left-width: 5px;
            border-left-color: turquoise;
            padding: 10px;
            width: 300px;
        }
        
        .embed img {
            float: left;
            margin-left: 5px;
            border-radius: 50%;
        }
        
        .reactions {
            text-align: center;
            color: lightgray;
            background-color: rgba(0, 0, 0, 0.3);
            border-radius: 3%;
            width: 22px;
            margin-top: 2px;
            margin-bottom: 2px;
            padding: 5px;
        }
        
        .attachment {
            height: 200px;
            width: 200px;
        }
        
        td {
            word-wrap: break-word;
        }
        
        th {
            text-align: left;
        }
        
        .thumbnail {

        }
        
        .time {
            color: #aaaaaa;
        }
    </style>
</head>

<body> ''')

        msg: Template = Template('''
            <div class="container">
                <img class="authorimage" src="{{author_image}}">
                <table>
                    <tr>
                        <th class="authorname" style="color:{{author_color}}">{{author}}:</th>
                    </tr>
                    <tr>
                        {{timestamp}}
                    </tr>
                    <tr>
                        {{message_content}}
                    </tr>
        ''')

        image: Template = Template('''
                    <tr>
                        {{image}}
                    </tr>
        ''')

        endmessage: str = '''
        
                </table>
            </div>
        
        '''

        embed: Template = Template('''
    <div class="container">
        <img class="authorimage" src="{{author_image}}">
        <table>
            <tr>
                <th class="authorname" style="color:{{author_color}}">{{author}}:
                </th>
            </tr>
            <tr class = title>
                <td>{{title}}</td>
            </tr>
            <tr>
                <td> {{description}}
                </td>
            </tr>
            <tr>
                <img class = "footerimg" src = "{{footerimg}}"><td class="time">{{footer}}</td>
            </tr>
        </table>
    </div>
    
    ''')
        reactions: Template = Template('''
    <td>
        {{reacions}}
    </td>                       
            ''')
        end = '''    
</body>
</html>
        '''
        h: str = head.render(
            user=channel.name.split('-')[0],
        )
        lst: list[str] = [h]
        histo: HistoryIterator = channel.history(limit=1000, oldest_first=True)
        history: list[Message] = await histo.flatten()
        message: Message
        for message in history:
            m = msg.render(
                author_image=message.author.avatar_url,
                author=f'{message.author.display_name}#{message.author.discriminator}',
                timestamp=message.created_at,
                message_content=message.content
            )

            lst.append(m)
            attachment: Attachment
            for attachment in message.attachments:
                img = image.render(image=f'<img src = "{attachment.url}" class = "attachment" /><br/>')
                lst.append(img)
            embeds: Embed
            lst.append(endmessage)

            for embeds in message.embeds:
                em = embed.render(
                    author_image=embeds.author.url if embeds.author.url != Embed.Empty else '',
                    author=embeds.author.name if embeds.author != Embed.Empty else '',
                    author_color=message.author.color,
                    title=embeds.title,
                    thumbnail=embeds.thumbnail.url if embeds.thumbnail != Embed.Empty else '',
                    description=embeds.description,
                    image=embeds.image.url if embeds.image != Embed.Empty else '',
                    footer_image=embeds.footer.url if embeds.footer.url != '' else '',
                    footer=embeds.footer.text if embeds.footer != Embed.Empty else '',
                )
                lst.append(em)

            for reaction in message.reactions:
                emoji = reaction.emoji if (type(reaction.emoji) == str) else reaction.emoji.url
                lst.append(reactions.render(reaction=emoji))

        lst.append(end)
        transcript = ''.join(lst)
        sio = StringIO()
        sio.write(transcript)
        sio.seek(0)
        transcripts = await self.bot.fetch_channel(self.bot.channels['transcripts'])
        await transcripts.send(file=File(fp=sio, filename=f'./{channel.name}.html'))
        sio.close()
        try:
            remove(f'./{channel.name}.html')
        except FileNotFoundError:
            pass
        await channel.send('Transcript successfully saved.')
        return transcript

    @command()
    @has_any_role('Owner', 'Head Admin', 'Admin', 'Head Mod', 'Moderator', 'Trial Mod')
    async def close(self, ctx: Context):
        await ctx.message.delete()
        return await self.close_ticket(ctx.channel, ctx.author)

    @has_any_role('Owner', 'Head Admin', 'Admin', 'Head Mod', 'Moderator', 'Trial Mod')
    @command()
    async def open(self, ctx: Context):
        await ctx.message.delete()
        return await self.reopen_ticket(ctx.channel, ctx.author)

    @has_any_role('Owner', 'Head Admin', 'Admin', 'Head Mod', 'Moderator', 'Trial Mod')
    @command()
    async def transcript(self, ctx: Context):
        await ctx.message.delete()
        return await self.ticket_transcript(ctx.channel, ctx.author)

    @has_any_role('Owner', 'Head Admin', 'Admin', 'Head Mod', 'Moderator', 'Trial Mod')
    @command(aliases=['remove'])
    async def delete(self, ctx: Context):
        await ctx.message.delete()
        report: Embed = Embed(title='Deleted', description='Ticket will be deleted in 5 seconds.', color=0xff0000,
                              timestamp=datetime.utcnow())
        await ctx.send(embed=report)
        await sleep(5)
        await self.delete_ticket(ctx.channel)
    

    @Cog.listener()
    async def on_raw_reaction_add(self, payload: RawReactionActionEvent):
        if payload.member.bot:
            return
        await self.bot.wait_until_ready()
        while self.lock:
            await sleep(1)
        if payload.message_id in [self.sverify.id, self.vverify.id] and str(
                payload.emoji) == '🎟':
            await self.sverify.remove_reaction('🎟', payload.member)
            await self.vverify.remove_reaction('🎟', payload.member)
            async with self._lock:
                category: CategoryChannel = self.verificationticket
                connection: Connection
                async with self.bot.pool.acquire() as connection:
                    async with connection.transaction():
                        ticketid = await connection.fetchval('SELECT id FROM tickets WHERE authorid = $1',
                                                            payload.member.id)
                        if ticketid is None:
                            await self.create_ticket(category, payload.member)
                        else:
                            await payload.member.send(
                                'Ticket limit reached 1/1. Please wait until your current ticket is closed and deleted.')

        elif payload.message_id in [self.rep.id, self.gen.id, self.apl.id] and str(
                payload.emoji) == '🎟':
            await self.rep.remove_reaction('🎟', payload.member)
            await self.gen.remove_reaction('🎟', payload.member)
            await self.apl.remove_reaction('🎟', payload.member)
            async with self._lock:
                category: CategoryChannel = self.generalticket
                connection: Connection
                async with self.bot.pool.acquire() as connection:
                    async with connection.transaction():
                        ticketid = await connection.fetchval('SELECT authorid FROM tickets WHERE authorid = $1', payload.user_id)
                        if ticketid is None:
                            await self.create_ticket(category, payload.member)
                        else:
                            await payload.member.send(
                                'Ticket limit reached 1/1. Please wait until your current ticket is closed and deleted.')

        elif payload.message_id == self.app.id and str(payload.emoji) == '🎟':
            await self.app.remove_reaction('🎟', payload.member)
            async with self._lock:
                category: CategoryChannel = self.applyticket
                connection: Connection
                async with self.bot.pool.acquire() as connection:
                    async with connection.transaction():
                        ticketid = await connection.fetchval('SELECT authorid FROM tickets WHERE authorid = $1',
                                                            payload.member.id)
                        if ticketid is None:
                            await self.create_ticket(category, payload.member)
                        else:
                            await payload.member.send(
                                'Ticket limit reached 1/1. Please wait until your current ticket is closed and deleted.')
        else:
            return

    @Cog.listener('on_raw_reaction_add')
    async def on_reactions_click(self, payload: RawReactionActionEvent):
        await self.bot.wait_until_ready()
        if payload.member.bot or str(payload.emoji) not in ['🔥', '📜', '🔓', '🔒']:
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                authorid = await connection.fetchval('SELECT authorid FROM tickets WHERE channelid = $1', payload.channel_id, )
        if authorid is None:
            return
        channel: TextChannel = self.bot.get_channel(payload.channel_id)
        hist: HistoryIterator = channel.history(limit=None, oldest_first=True)
        history: list[Message] = await hist.flatten()
        embeds: list[Message] = []
        for message in history:
            if message.author.id == self.bot.user.id:
                embeds.append(message)
        message: Message = await channel.fetch_message(payload.message_id)
        if message not in embeds:
            return
        for reaction in message.reactions:
            users: list[User] = await reaction.users().flatten()
            if str(reaction.emoji) in ['🔥', '📜', '🔓', '🔒'] and self.bot.user in users:
                if str(payload.emoji) == '🔥':
                    await message.remove_reaction('🔥', payload.member)
                    report: Embed = Embed(title='Deleted', description='Ticket will be deleted in 5 seconds.', color=0xff0000,
                              timestamp=datetime.utcnow())
                    await channel.send(embed=report)
                    await sleep(5)
                    await self.delete_ticket(channel)
                elif str(payload.emoji) == '📜':
                    await message.remove_reaction('📜', payload.member)
                    await self.ticket_transcript(channel, payload.member)
                elif str(payload.emoji) == '🔓':
                    await message.remove_reaction('🔓', payload.member)
                    await self.reopen_ticket(channel, payload.member)
                else:
                    await message.remove_reaction('🔒', payload.member)
                    await self.close_ticket(channel, payload.member)

                return
    @Cog.listener()
    async def on_member_remove(self, member: Member):
        if member.bot:
            return
        await self.bot.wait_until_ready()
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                channelid = await connection.fetchval('SELECT channelid FROM tickets WHERE authorid = $1', member.id, )
        if channelid is None:
            return
        channel: TextChannel = self.bot.get_channel(channelid)
        await self.delete_ticket(channel)
    
    @Cog.listener()
    async def on_member_ban(self, guild: Guild, member: Union[User, Member]):
        if member.bot:
            return
        await self.bot.wait_until_ready()
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                channelid = await connection.fetchval('SELECT channelid FROM tickets WHERE authorid = $1', member.id, )
        if channelid is None:
            return
        channel: TextChannel = guild.get_channel(channelid)
        await self.delete_ticket(channel)

    @has_any_role('Owner', 'Head Admin', 'Admin', 'Head Mod', 'Moderator', 'Trial Mod')
    @command()
    async def claim(self, ctx: Context):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id = await connection.fetchval('SELECT authorid FROM tickets WHERE channelid = $1', ctx.channel.id, )
                if id is None:
                    await ctx.send('This command can only be used on a ticket.')
                else:
                    guild: Guild = await self.bot.fetch_guild(812314425318440961)
                    member: Member = await guild.fetch_member(id)
                    headmod: Role = guild.get_role(813511889476386826)
                    mod: Role = guild.get_role(812315517511008307)
                    trialmod: Role = guild.get_role(812318396849848341)
                    overwrite = {
                        guild.default_role: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        headmod: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        mod: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        trialmod: PermissionOverwrite(
                            read_messages=False,
                            view_channel=False,
                            send_messages=False,
                            embed_links=False,
                            attach_files=False
                        ),
                        member: PermissionOverwrite(
                            read_messages=True,
                            view_channel=True,
                            send_messages=True,
                            embed_links=True,
                            attach_files=True
                        ),
                        ctx.author: PermissionOverwrite(
                            read_messages=True,
                            view_channel=True,
                            send_messages=True,
                            embed_links=True,
                            attach_files=True
                        )
                    }
                    claimed: Embed = Embed(title='Claimed', color=0x00ff00, timestamp=datetime.utcnow(),
                                           description=f'Ticket claimed by {ctx.author.mention}.')
                    await ctx.channel.edit(overwrites=overwrite)
                    await ctx.send(embed=claimed)


def setup(bot: Bot):
    bot.add_cog(Ticket(bot))

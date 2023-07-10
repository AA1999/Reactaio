from asyncio.tasks import wait_for
from datetime import datetime

import discord
from asyncpg.connection import Connection
from discord.channel import TextChannel
from discord.embeds import Embed
from discord.ext import commands
from discord.ext.commands import Context
from discord.ext.commands.bot import Bot
from discord.ext.commands.cog import Cog
from discord.ext.tasks import loop
from discord.member import Member, VoiceState


class Levels(Cog):
    bot: Bot

    # text channels
    staff_room = 812369644152422440
    staff_activity = 827300967573356554
    staff_issues = 824070693712560158
    council_meeting = 825640284473262080
    introductions = 812913559847305267
    events = 816870454778134539
    general = 812314425318440969
    verified_lounge = 813108084330987520
    truth_or_dare = 837179922048745502
    vent = 812488484617977886
    vent_no_response = 828875733580382238
    ask_to_dm = 812948187588460564
    rp = 826852369178296351
    suggestions = 828848514054094888
    female_only = 822614108100362271
    bois = 827107245468680194
    bot_request = 812467901390127146
    complaints = 826887289233342464
    confessions_reactions = 813004709458870292
    selfies = 812495695494774814
    selfies_reactions = 822234403652501604
    media = 812487796102135818
    media_reactions = 823192462767816724
    writings = 826416489003352085
    art_room = 812477606968033320
    nsfw = 812497772069912588
    horny_jail = 821872432028844103
    nsfw_male = 814634822110674985
    nsfw_female = 812498125322584095
    verified_vc = 827214077046292531
    gean_simps = 826816336465494046
    nev_moonbeams = 823939808120274944
    panther_inspiration = 827095861044576266
    arshia_idea_dump = 827161540918640670
    vc_text = 812986121461825556

    text_channels = [
        staff_room, staff_activity, staff_issues, council_meeting, introductions, events, general, verified_lounge,
        truth_or_dare,
        vent, vent_no_response, ask_to_dm, rp, suggestions, female_only, bois, bot_request, complaints,
        confessions_reactions,
        selfies, selfies_reactions, media, media_reactions, writings, art_room, nsfw, horny_jail, nsfw_male,
        nsfw_female,
        verified_vc, gean_simps, nev_moonbeams, panther_inspiration, arshia_idea_dump, vc_text
    ]

    # voice channels
    council_meeting_voice = 820514912744898591
    join_to_create = 823993497278349372
    lounge_voice = 812506797092896829
    verified_voice = 813108490654580756
    public_i = 826848694540042301
    public_ii = 827881646392213514
    public_iii = 827881692685271060
    private_i = 826848752891461733
    private_ii = 826848849720508446
    private_iii = 828852588283559946
    music_i = 827221843617251338
    music_ii = 827221921799864411
    music_iii = 827221970625101914
    stream_1 = 816870513660919838
    stream_2 = 816870573116227584
    among_us = 816870764770230283
    cah = 828853568026574849
    jack_box = 816870693588435034
    scribblo = 816870764770230283
    movie_night = 816870944740081694
    karakoe = 816871092735442965
    show = 828707826896863243
    voice_1v1 = 828707922623070228
    waiting = 828707985886543922

    level_up = 812549913820790784

    voice_channels = [council_meeting_voice, join_to_create, lounge_voice, verified_voice, public_i, public_ii,
                      public_iii, private_i, private_ii, private_iii, music_i, music_ii, music_iii, stream_1, stream_2,
                      among_us, cah, scribblo, jack_box, movie_night, karakoe, show, voice_1v1, waiting]

    async def setup(self):
        await self.bot.wait_until_ready()
        self.reactaiolevelups: TextChannel = self.bot.get_channel(self.bot.channels['reactaio-rank'])

    def __init__(self, bot: Bot):
        self.bot = bot

    @Cog.listener()
    async def on_ready(self):
        print('Leveling system active!\n')

    @Cog.listener()
    async def on_message(self, message: discord.Message):
        if message.channel.id not in self.text_channels or message in self.bot.commands or message.author.bot:
            return
        await wait_for(self.setup(), timeout=None)
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                text = await connection.fetchrow('SELECT textxp, textlevel FROM levels WHERE memberid = $1',
                                                 message.author.id, )
                if text is not None:
                    xp: int = text['textxp']
                    oldlevel: int = text['textlevel']
                    formula = int((xp + 5) / (10 * (2 ** oldlevel)))
                    newlevel: int = formula if formula > 0 else 1
                    if newlevel > oldlevel and newlevel != 1:
                        levelup: Embed = Embed(title='Level Up!', color=0xff0000, timestamp=datetime.utcnow(),
                                               description=f'**{str(message.author)}** has leveled up to level '
                                                           f'{newlevel}! Congrats!',
                                               thumbnail=message.author.avatar_url)
                        levelup.set_footer(text=f'Use **{self.bot.command_prefix}rank**'
                                                f' to view your rank and **{self.bot.command_prefix}leaderboard**'
                                                f' to view the ranks of others.')
                        await self.reactaiolevelups.send(embed=levelup)
                    await connection.execute('UPDATE levels SET textxp = $1, textlevel = $2 WHERE memberid = $3'
                                             , xp + 5, newlevel, message.author.id, )
                else:
                    roles: list[int] = []
                    for role in message.author.roles:
                        roles.append(role.id)
                    await connection.execute('INSERT INTO levels (memberid, textxp, textlevel,'
                                             ' voicexp, voicelevel) VALUES($1, 0, 1, 0, 1)', message.author.id)

    @loop(seconds=1)
    async def countdown(self, member: Member):
        if member.voice == True:
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    xp = await connection.fetchrow('SELECT voicexp, voicelevel FROM levels WHERE memberid = $1',
                                                   (member.id), )
                    oldlevel = xp['voicelevel']
                    oldxp = xp['voicexp']
                    newlevel: int = (oldxp + 5) / (100 * (2 ** oldlevel)) if oldlevel != 1 else xp['voicexp'] / 100
                    if newlevel > oldlevel != 0:
                        levelup: Embed = Embed(title='Level Up!', color=0xff0000, timestamp=datetime.utcnow(),
                                               description=f'**{str(member)}** has leveled up to level {newlevel}! Congrats!')
                        levelup.set_thumbnail(url=member.avatar_url)
                        levelup.set_footer(
                            text=f'Use {self.bot.command_prefix}rank to view your rank and {self.bot.command_prefix}top to view the ranks of others.')
                        await self.reactaiolevelups.send(embed=levelup)
                        await connection.execute(
                            'UPDATE levels SET textxp = $1, textlevel = $2 WHERE memberid = $3',
                            xp['textxp'] + 5, newlevel, member.id, )

    @countdown.before_loop
    async def before_countdown(self):
        await self.bot.wait_until_ready()

    @Cog.listener()
    async def on_voice_state_update(self, member: Member, before: VoiceState, after: VoiceState):
        if member.bot:
            return
        if member.voice:
            self.countdown.start(member=member)
        else:
            self.countdown.stop()

    @commands.command()
    async def rank(self, ctx: Context):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                level = await connection.fetchrow(
                    'SELECT memberid, textxp, textlevel, voicexp, voicelevel FROM levels WHERE memberid = $1',
                    (ctx.author.id), )
                rank: Embed = Embed(title=f'{ctx.author}\'s Rank', color=0x00ff00, timestamp=datetime.utcnow(),
                                    thumbnail=ctx.author.avatar)
                rank.add_field(name='Text Rank: ', value=f'{level["textxp"]} XP, Level {level["textlevel"]}',
                               inline=False)
                rank.add_field(name='Voice Rank: ', value=f'{level["voicexp"]} XP, Level {level["voicelevel"]}',
                               inline=False)
                await ctx.send(embed=rank)

    @commands.command(aliases=['leaderboard', 'lead'])
    async def top(self, ctx: Context):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                text = await connection.fetch('SELECT memberid, textxp, textlevel FROM levels ORDER BY textlevel DESC')
                voice = await connection.fetch(
                    'SELECT memberid, voicexp, voicelevel FROM members ORDER BY voicelevel DESC')
                field: str = '**Text ranks**\n\n'
                vfield: str = '**Voice Ranks**\n\n'
                rank: int = 1
                for txt in text:
                    field += f'**#{rank}: ** XP: **{txt["textxp"]}, **Level: **{txt["textlevel"]}\n'
                    rank += 1
                for rank, vc in enumerate(voice, start=1):
                    vfield += F'**#{rank}: **XP: **{vc["voicexp"]}, **Level: **{vc["voicelevel"]}\n'
                await ctx.send(field)
                await ctx.send('\n\n\n\n')
                await ctx.send(vfield)


def setup(bot: Bot):
    bot.add_cog(Levels(bot))

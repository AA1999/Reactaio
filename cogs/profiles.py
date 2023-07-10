from datetime import datetime, timedelta
from asyncio.tasks import sleep
from asyncpg import Connection
from discord import Member, Message, Forbidden, Embed, Colour, TextChannel, Attachment, Reaction
from discord.ext.commands import Cog, Bot, command, Context

from discord.guild import Guild
from discord.raw_models import RawReactionActionEvent
from discord.role import Role


class Profile(Cog):
    bot: Bot
    lock: bool

    async def check(self, message: Message):
        return message not in self.pins

    async def config(self):
        await self.bot.wait_until_ready()
        profiles: TextChannel = self.bot.get_channel(self.bot.channels['profiles'])
        self.pins = await profiles.pins()
        await profiles.purge(limit=None, check=self.check)
        reactaio: TextChannel = self.bot.get_channel(self.bot.channels['reactaio'])
        ui: Embed = Embed(title='Profile Manager', color=0xff0000, timestamp=datetime.utcnow())
        ui.description = f'React with 📝 to edit the profile and 📢 to bump the profile.\nUse ' \
                         f'{self.bot.command_prefix}start in {reactaio.mention} to create a new' \
                         f' profile. You can also use {self.bot.command_prefix}edit in the same' \
                         f' channel to edit a message and {self.bot.command_prefix}bump to bump your profile.'
        msg: Message = await profiles.send(embed=ui)
        self.bot.message = msg
        await msg.add_reaction('📝')
        await msg.add_reaction('📢')

        self.lock = False

    def __init__(self, bot: Bot):
        self.bot = bot
        self.lock = True
        self.bot.loop.create_task(self.config())

    @Cog.listener()
    async def on_raw_reaction_add(self, payload: RawReactionActionEvent):
        await self.bot.wait_until_ready()
        guild: Guild = self.bot.get_guild(payload.guild_id)
        member: Member = guild.get_member(payload.user_id)
        if member.bot:
            return
        while self.lock:
            await sleep(1)
        if payload.message_id == self.bot.message.id:
            if str(payload.emoji) == '📝':
                try:
                    guild: Guild = self.bot.get_guild(payload.guild_id)
                    member: Member = guild.get_member(payload.user_id)
                    await self.edit_profile(member)
                except Forbidden:
                    pass
            elif str(payload.emoji) == '📢':
                try:
                    guild: Guild = self.bot.get_guild(payload.guild_id)
                    member: Member = guild.get_member(payload.user_id)
                    code = await self.bump_profile(member)
                    if code == -1:
                        await self.bot.message.remove_reaction('📢', member)
                except Forbidden:
                    pass

    @command()
    async def bump(self, ctx: Context):
        await self.bot.wait_until_ready()
        if ctx.guild is None or ctx.channel.id != self.bot.channels['reactaio']:
            return
        guild: Guild = ctx.guild
        member: Member = ctx.author
        try:
            await self.bump_profile(member)
        except Forbidden:
            pass

    @command()
    async def start(self, ctx: Context):
        await self.bot.wait_until_ready()
        if ctx.guild is None or ctx.channel.id != self.bot.channels['reactaio']:
            return
        try:
            await self.create_profile(ctx.author)
        except Forbidden:
            await ctx.send(
                'Unable to send message to user, please make sure the bot isn\'t blocked and/or you allow DMs form '
                'server members in your settings.')

    async def bump_profile(self, member: Member):

        guild: Guild = member.guild

        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                lastbump: datetime = await connection.fetchval('SELECT lastbump FROM profiles WHERE id = $1',
                                                               member.id, )

        if lastbump is not None:
            delta: timedelta = datetime.now(lastbump.tzinfo) - lastbump
            if delta.days < 2:
                delta = datetime(2021, 1, 3, 0, 0, 0, 0, lastbump.tzinfo) - datetime(2021, 1, 1, 0, 0, 0, 0,
                                                                                     lastbump.tzinfo) - delta
                hours, r = divmod(delta.seconds, 3600)
                minutes, secs = divmod(r, 60)
                await member.send(
                    f'You need to wait {delta.days} days, {hours} hours, {minutes} minutes and {secs} seconds until '
                    'next bump.')
                return -1

        def find_which_role(*roles):
            for role in roles:
                if role in member.roles:
                    return role
            return None

        def has_any_role(*roles):
            for role in roles:
                if role in member.roles:
                    return True
            return False

        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id = await connection.fetchval('SELECT id FROM profiles WHERE id = $1', member.id)
                if id is None:
                    reactaio: TextChannel = await self.bot.fetch_channel(self.bot.channels['reactaio'])
                    try:
                        await member.send(
                            f'You don\'t have any profile. Use {self.bot.command_prefix}start in {reactaio.mention} to create a new profile.')
                        return
                    except Forbidden:
                        pass

                embed = await connection.fetchrow('SELECT * FROM profiles WHERE id = $1', member.id, )
        male: Role = guild.get_role(812683252589658132)
        ftm: Role = guild.get_role(822147501591101540)
        female: Role = guild.get_role(812683209171140628)
        mtf: Role = guild.get_role(822147395311370280)
        gf: Role = guild.get_role(813437299366428754)
        nb: Role = guild.get_role(812683265919942667)
        bg: Role = guild.get_role(825728975305310270)
        verifiedm: Role = guild.get_role(813436212655554580)
        verifiedftm: Role = guild.get_role(822181510140526633)
        verifiedf: Role = guild.get_role(813436551417430116)
        verifiedmtf: Role = guild.get_role(822181708396363836)
        verifiednb: Role = guild.get_role(813438399872106568)
        verifiedgf: Role = guild.get_role(813438134301884417)
        role: Role = find_which_role(male, ftm, female, mtf, nb, gf, bg)
        if role == male:
            channel: TextChannel = guild.get_channel(self.bot.channels['👨-male-profiles'])
        elif role == female:
            channel: TextChannel = guild.get_channel(self.bot.channels['👩-female-profiles'])
        else:
            channel: TextChannel = guild.get_channel(self.bot.channels['🧑-other-profiles'])
        message: Message = await channel.fetch_message(embed['embedid'])
        profile: Embed = Embed(title=f'{member.display_name}\'s Profile', color=message.embeds[0].color)
        profile.add_field(name='Discord username: ', value=member.mention, inline=True)
        profile.add_field(name='Name: ', value=embed['name'], inline=True)
        profile.add_field(name='Age: ', value=str(embed['age']), inline=True)
        profile.add_field(name='Gender: ', value=embed['gender'], inline=False)
        profile.add_field(name='Location: ', value=embed['location'], inline=True)
        profile.add_field(name='Sexuality: ', value=embed['sexuality'], inline=True)
        profile.add_field(name='Looking for: ', value=embed['seeking'], inline=False)
        profile.add_field(name='Hobbies: ', value=embed['hobbies'], inline=True)
        profile.add_field(name='Likes: ', value=embed['likes'], inline=True)
        profile.add_field(name='Dislikes: ', value=embed['dislikes'], inline=True)
        if embed['about'] is not None:
            profile.add_field(name='About me: ', value=embed['about'], inline=False)
        if embed['selfie'] is not None:
            profile.set_image(url=embed['selfie'])
        profile.add_field(name='Verified: ', value='Yes' if has_any_role(verifiedm, verifiedftm, verifiedf,
                                                                         verifiedmtf, verifiedgf,
                                                                         verifiednb) else 'No')
        profile.set_thumbnail(url=member.avatar_url)
        await member.send('Profile bumped successfully!')
        await channel.send(f'{member.mention}, ', embed=profile)
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.execute('UPDATE profiles SET lastbump = $1 WHERE id = $2', datetime.utcnow(),
                                         member.id, )
        return 0

    async def edit_profile(self, member: Member):
        guild: Guild = member.guild
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id = await connection.fetchval('SELECT id FROM profiles WHERE id = $1', member.id)
                if id is None:
                    reactaio: TextChannel = await self.bot.fetch_channel(self.bot.channels['reactaio'])
                    await member.send(
                        f'You don\'t have any profile. Use {self.bot.command_prefix}start in {reactaio.mention} to create a new profile.')
                    return
        message: Message = await member.send('''Please choose one of the following types to edit:\n
                            1. Name\n
                            2. Age\n
                            3. Location\n
                            4. Looking for\n
                            5. Hobbies\n
                            6. Likes\n
                            7. Dislikes\n
                            8. About yourself\n
                            9. Update/Add selfie(verified only)\n
                            0. Exit\n
        ''')
        await message.add_reaction('1️⃣')
        await message.add_reaction('2️⃣')
        await message.add_reaction('3️⃣')
        await message.add_reaction('4️⃣')
        await message.add_reaction('5️⃣')
        await message.add_reaction('6️⃣')
        await message.add_reaction('7️⃣')
        await message.add_reaction('8️⃣')
        await message.add_reaction('9️⃣')
        await message.add_reaction('0️⃣')

        def check(reaction: Reaction, user: Member):
            return str(reaction.emoji) in ['1️⃣', '2️⃣', '3️⃣', '4️⃣', '5️⃣', '6️⃣', '7️⃣', '8️⃣', '9️⃣', '0️⃣'] and \
                   user == member

        reaction: Reaction
        user: Member
        reaction, user = await self.bot.wait_for('reaction_add', check=check)
        verifiedm: Role = guild.get_role(813436212655554580)
        verifiedftm: Role = guild.get_role(822181510140526633)
        verifiedf: Role = guild.get_role(813436551417430116)
        verifiedmtf: Role = guild.get_role(822181708396363836)
        verifiednb: Role = guild.get_role(813438399872106568)
        verifiedgf: Role = guild.get_role(813438134301884417)

        def find_which_role(*roles):
            for role in roles:
                if role in member.roles:
                    return role
            return None

        def check1(message: Message):
            return message.author == member and message.guild is None and all(
                c.isalpha() or c.isspace() for c in message.content)

        def isdigit(message: Message):
            return message.content.isdigit() and message.author == member

        def check2(message: Message):
            return member == message.author and message.guild is None and all(
                c.isalpha() or c.isspace() or c == ',' for c in message.content)

        def check3(message: Message):
            return member == message.author and message.guild is None and all(
                c.isalnum() or c.isspace() or c == ',' or c == '.' or c == ';' or c == ':' or c == '\n' for c in
                message.content)

        def has_any_role(*roles):
            for role in roles:
                if role in member.roles:
                    return True
            return False

        if str(reaction.emoji) == '1️⃣':
            await member.send('Please enter the new name: ')
            msg: Message = await self.bot.wait_for('message', check=check1)
            name: str = msg.content
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    embed = await connection.fetchval('UPDATE profiles SET name = $1 WHERE id = $2 RETURNING *',
                                                      name, member.id, )
        elif str(reaction.emoji) == '2️⃣':
            await member.send('Please enter the new age: ')
            msg: Message = await self.bot.wait_for('message', check=isdigit)
            age = int(msg.content)
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    embed = await connection.fetchval('UPDATE profiles SET age = $1 WHERE id = $2 RETURNING *',
                                                      age, member.id, )
        elif str(reaction.emoji) == '4️⃣':
            await member.send('Please enter the new Looking for: ')
            msg: Message = await self.bot.wait_for('message', check=check2)
            seeking: str = msg.content
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    embed = await connection.fetchval('UPDATE profiles SET seeking = $1 WHERE id = $2 RETURNING *',
                                                      seeking, member.id, )

        elif str(reaction.emoji) == '5️⃣':
            await member.send('Please enter the new hobbies: ')
            msg: Message = await self.bot.wait_for('message', check=check2)
            hobbies: str = msg.content
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    embed = await connection.fetchval('UPDATE profiles SET hobbies = $1 WHERE id = $2 RETURNING *',
                                                      hobbies, member.id, )

        elif str(reaction.emoji) == '6️⃣':
            await member.send('Please enter the new likes: ')
            msg: Message = await self.bot.wait_for('message', check=check2)
            hobbies: str = msg.content
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    embed = await connection.fetchval('UPDATE profiles SET hobbies = $1 WHERE id = $2 RETURNING *',
                                                      hobbies, member.id, )
        elif str(reaction.emoji) == '7️⃣':
            await member.send('Please enter the new dislikes: ')
            msg: Message = await self.bot.wait_for('message', check=check2)
            likes: str = msg.content
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    embed = await connection.fetchval('UPDATE profiles SET likes = $1 WHERE id = $2 RETURNING *',
                                                      likes, member.id, )
        elif str(reaction.emoji) == '8️⃣':
            await member.send('Please tell us about yourself: ')
            msg: Message = await self.bot.wait_for('message', check=check2)
            dislikes: str = msg.content
            connection: Connection
            async with self.bot.pool.acquire() as connection:
                async with connection.transaction():
                    embed = await connection.fetchval('UPDATE profiles SET dislikes = $1 WHERE id = $2 RETURNING *',
                                                      dislikes, member.id, )
        elif str(reaction.emoji) == '9️⃣':
            if has_any_role(verifiedm, verifiedftm, verifiedf, verifiedmtf, verifiedgf, verifiednb):
                await member.send('Please upload a selfie: ')
                msg: Message = await self.bot.wait_for('message', check=check3)
                about: str = msg.content
                connection: Connection
                async with self.bot.pool.acquire() as connection:
                    async with connection.transaction():
                        embed = await connection.fetchval('UPDATE profiles SET about = $1 WHERE id = $2 RETURNING *',
                                                          about, member.id, )
        elif str(reaction.emoji) == '0️⃣':
            return

        male: Role = guild.get_role(812683252589658132)
        ftm: Role = guild.get_role(822147501591101540)
        female: Role = guild.get_role(812683209171140628)
        mtf: Role = guild.get_role(822147395311370280)
        gf: Role = guild.get_role(813437299366428754)
        nb: Role = guild.get_role(812683265919942667)
        bg: Role = guild.get_role(825728975305310270)
        verifiedm: Role = guild.get_role(813436212655554580)
        verifiedftm: Role = guild.get_role(822181510140526633)
        verifiedf: Role = guild.get_role(813436551417430116)
        verifiedmtf: Role = guild.get_role(822181708396363836)
        verifiednb: Role = guild.get_role(813438399872106568)
        verifiedgf: Role = guild.get_role(813438134301884417)
        role: Role = find_which_role(male, ftm, female, mtf, nb, gf, bg)
        if role == male:
            channel: TextChannel = guild.get_channel(self.bot.channels['👨-male-profiles'])
        elif role == female:
            channel: TextChannel = guild.get_channel(self.bot.channels['👩-female-profiles'])
        else:
            channel: TextChannel = guild.get_channel(self.bot.channels['🧑-other-profiles'])
        pro: Message = await channel.fetch_message(embed['embedid'])
        name = embed['name']
        age = embed['age']
        gender = embed['gender']
        location = embed['location']
        sexuality = embed['sexuality']
        seeking = embed['seeking']
        hobbies = embed['hobbies']
        likes = embed['likes']
        dislikes = embed['dislikes']
        about = embed['about']
        selfie = embed['selfie']
        profile: Embed = Embed(title=f'{member.display_name}\'s Profile', color=pro.embeds[0].color,
                               timestamp=pro.embeds[0].timestamp)
        profile.add_field(name='Discord username: ', value=member.mention, inline=True)
        profile.add_field(name='Name: ', value=name, inline=True)
        profile.add_field(name='Age: ', value=str(age), inline=True)
        profile.add_field(name='Gender: ', value=gender, inline=False)
        profile.add_field(name='Location: ', value=location, inline=True)
        profile.add_field(name='Sexuality: ', value=sexuality, inline=True)
        profile.add_field(name='Looking for: ', value=seeking, inline=False)
        profile.add_field(name='Hobbies: ', value=hobbies, inline=True)
        profile.add_field(name='Likes: ', value=likes, inline=True)
        profile.add_field(name='Dislikes: ', value=dislikes, inline=True)
        profile.add_field(name='Verified: ', value='Yes' if has_any_role(verifiedm, verifiedftm, verifiedf,
                                                                         verifiedmtf, verifiedgf,
                                                                         verifiednb) else 'No')
        if about is not None:
            profile.add_field(name='About me: ', value=about)
        if selfie is not None:
            profile.set_image(url=selfie)
        profile.set_thumbnail(url=member.avatar_url)
        await pro.edit(embed=profile)

    async def create_profile(self, member: Member):

        guild: Guild = member.guild
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id = await connection.fetchval('SELECT id FROM profiles WHERE id = $1', member.id)
                if id is not None:
                    profiles: TextChannel = guild.get_channel(self.bot.channels['profiles'])
                    try:
                        await member.send(
                            f'You already created a profile, use **{self.bot.command_prefix}edit** to edit it'
                            f'or head to {profiles.mention} to bump it.')
                        return
                    except Forbidden:
                        pass

        def has_any_role(*roles):
            for role in roles:
                if role in member.roles:
                    return True
            return False

        def find_which_role(*roles):
            for role in roles:
                if role in member.roles:
                    return role
            return None

        age18: Role = guild.get_role(812319496878293022)
        age23: Role = guild.get_role(812319684829380660)
        age28: Role = guild.get_role(812319769046548490)
        age33: Role = guild.get_role(812319896980422706)
        male: Role = guild.get_role(812683252589658132)
        ftm: Role = guild.get_role(822147501591101540)
        female: Role = guild.get_role(812683209171140628)
        mtf: Role = guild.get_role(822147395311370280)
        gf: Role = guild.get_role(813437299366428754)
        nb: Role = guild.get_role(812683265919942667)
        bg: Role = guild.get_role(825728975305310270)
        he: Role = guild.get_role(812683300069048373)
        she: Role = guild.get_role(812683285112684555)
        they: Role = guild.get_role(812683318277308417)
        straight: Role = guild.get_role(812671124218839060)
        gay: Role = guild.get_role(812671238711410709)
        lesbian: Role = guild.get_role(812671269468110868)
        bi: Role = guild.get_role(812671311901491220)
        pan: Role = guild.get_role(812671486322147330)
        asexual: Role = guild.get_role(812671399046545429)
        demisexual: Role = guild.get_role(822216571732099132)
        aromantic: Role = guild.get_role(823958614623518770)
        femsexual: Role = guild.get_role(839158077467000913)
        malesexual: Role = guild.get_role(839158165957509151)
        sapiosexual: Role = guild.get_role(839159974780993577)
        mono: Role = guild.get_role(822147565582680076)
        poly: Role = guild.get_role(822147642292174848)
        single: Role = guild.get_role(812658491742355457)
        taken: Role = guild.get_role(812658515154960404)
        complicated: Role = guild.get_role(812658536202371082)
        notlooking: Role = guild.get_role(816360293533614172)
        verifiedm: Role = guild.get_role(813436212655554580)
        verifiedftm: Role = guild.get_role(822181510140526633)
        verifiedf: Role = guild.get_role(813436551417430116)
        verifiedmtf: Role = guild.get_role(822181708396363836)
        verifiednb: Role = guild.get_role(813438399872106568)
        verifiedgf: Role = guild.get_role(813438134301884417)
        if (
                not has_any_role(age18, age23, age28, age33)
                or not has_any_role(male, ftm, female, mtf, gf, nb, bg)
                or not has_any_role(he, she, they)
                or not has_any_role(straight, gay, lesbian, bi, pan, asexual, demisexual, aromantic, femsexual,
                                    malesexual, sapiosexual)
                or not has_any_role(mono, poly)
                or not has_any_role(single, taken, complicated, notlooking)):
            await member.send('Age, gender, pronouns, sexuality, mono/poly and relationship roles are required to '
                              'make a profile.')
            return

        def check(message: Message):
            return message.author == member and message.guild is None and all(
                c.isalpha() or c.isspace() for c in message.content)

        def isdigit(message: Message):
            return message.content.isdigit() and message.author == member

        def check2(message: Message):
            return member == message.author and message.guild is None and all(
                c.isalpha() or c.isspace() or c == ',' for c in message.content)

        def check3(message: Message):
            return member == message.author and message.guild is None and all(
                c.isalnum() or c.isspace() or c == ',' or c == '.' or c == ';' or c == ':' or c == '\n' for c in
                message.content)

        def selfiecheck(message: Message):
            return message.attachments or ''.join(c for c in message.content if c.isalnum()).lower() == 'none'

        await member.send('Please enter your name: ')
        msg: Message = await self.bot.wait_for('message', check=check)
        name: str = msg.content
        await member.send('Please enter your age: ')
        ag: Message = await self.bot.wait_for('message', check=isdigit)
        age: int = int(ag.content)
        await member.send('Please type your location: ')
        loc: Message = await self.bot.wait_for('message', check=check)
        location: str = loc.content
        await member.send('Please state what you\'re looking for in the server: ')
        seek: Message = await self.bot.wait_for('message', check=check2)
        seeking: str = seek.content
        await member.send('Please share your hobbies: ')
        hob: Message = await self.bot.wait_for('message', check=check2)
        hobbies: str = hob.content
        await member.send('Please share your likes: ')
        lik: Message = await self.bot.wait_for('message', check=check2)
        likes: str = lik.content
        await member.send('Please share your dislikes: ')
        dis: Message = await self.bot.wait_for('message', check=check2)
        dislikes: str = dis.content
        await member.send('Please tell us about yourself(type **none** to skip): ')
        abt: Message = await self.bot.wait_for('message', check=check3)
        about: str = abt.content
        if has_any_role(verifiedm, verifiedftm, verifiedf, verifiedmtf, verifiedgf, verifiednb):
            await member.send('You may upload a selfie of yourself(type **none** to skip). ')
            selfie: Message = await self.bot.wait_for('message', check=selfiecheck)
            if ''.join(c for c in selfie.content if c.isalnum()).lower() != 'none':
                image: Attachment = selfie.attachments[0]
        await member.send('Profile creation completed! Check your mentions!')
        target: Role = find_which_role(straight, lesbian, gay, bi, pan, asexual, demisexual, aromantic, femsexual,
                                       malesexual, sapiosexual)
        sexuality = ''.join(c for c in target.name if c.isalnum()).removeprefix('Sexuality')
        target = find_which_role(male, ftm, female, mtf, nb, gf, bg)
        gender = ''.join(c for c in target.name if c.isalnum()).removeprefix('Gender')
        profile: Embed = Embed(title=f'{member.display_name}\'s Profile', color=Colour.random(),
                               timestamp=datetime.utcnow())
        profile.add_field(name='Discord username: ', value=member.mention, inline=True)
        profile.add_field(name='Name: ', value=name, inline=True)
        profile.add_field(name='Age: ', value=str(age), inline=True)
        profile.add_field(name='Gender: ', value=gender, inline=False)
        profile.add_field(name='Location: ', value=location, inline=True)
        profile.add_field(name='Sexuality: ', value=sexuality, inline=True)
        profile.add_field(name='Looking for: ', value=seeking, inline=False)
        profile.add_field(name='Hobbies: ', value=hobbies, inline=True)
        profile.add_field(name='Likes: ', value=likes, inline=True)
        profile.add_field(name='Dislikes: ', value=dislikes, inline=True)

        if ''.join(c for c in about if c.isalnum()).lower() != 'none':
            profile.add_field(name='About me: ', value=about, inline=False)
        profile.add_field(name='Verified: ', value='Yes' if has_any_role(verifiedm, verifiedftm, verifiedf,
                                                                         verifiedmtf, verifiedgf,
                                                                         verifiednb) else 'No')
        if 'image' in locals():
            profile.set_image(url=image.url)
        profile.set_thumbnail(url=member.avatar_url)
        role: Role = find_which_role(male, ftm, female, mtf, nb, gf, bg)
        if role == male:
            channel: TextChannel = guild.get_channel(self.bot.channels['👨-male-profiles'])
        elif role == female:
            channel: TextChannel = guild.get_channel(self.bot.channels['👩-female-profiles'])
        else:
            channel: TextChannel = guild.get_channel(self.bot.channels['🧑-other-profiles'])
        embed = await channel.send(f'{member.mention}, ', embed=profile)
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.execute('INSERT INTO profiles(id, name, age, gender, sexuality, location, seeking,'
                                         'hobbies, likes, dislikes, about, selfie, embedid, lastbump) VALUES($1, $2, '
                                         '$3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14)', member.id, name, age,
                                         gender, sexuality, location, seeking, hobbies, likes, dislikes, about,
                                         image.url if 'image' in locals() else None, embed.id, datetime.utcnow())

    @command()
    async def edit(self, ctx: Context):
        if ctx.guild is None or ctx.channel.id != self.bot.channels['reactaio']:
            return
        guild: Guild = ctx.guild
        member: Member = ctx.author
        try:
            await self.edit_profile(member)
        except Forbidden:
            await ctx.send(
                'Unable to send message to user, please make sure the bot isn\'t blocked and/or you allow DMs form '
                'server members in your settings.')


def setup(bot: Bot):
    bot.add_cog(Profile(bot))

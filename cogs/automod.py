import json
import re
from difflib import SequenceMatcher

from asyncpg.connection import Connection
from discord import Role, Forbidden
from discord.ext.commands.bot import Bot
from discord.ext.commands.cog import Cog
from discord.ext.commands.context import Context
from discord.ext.commands.cooldowns import BucketType, CooldownMapping
from discord.ext.commands.core import Command, command, has_any_role
from discord.ext.tasks import loop
from discord.guild import Guild
from discord.member import Member
from discord.message import Message
from aiohttp import ClientSession


async def reveal_short_url(url: str):
    async with ClientSession() as cs:
        async with cs.get(url) as u:
            return u.url


class Automod(Cog):
    bot: Bot

    @loop(seconds=15)
    async def autopunishment(self):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                guildids = await connection.fetch('SELECT guildid FROM warnings')
                for guildid in guildids:
                    guild: Guild = self.bot.get_guild(guildid)
                    count = await connection.fetchval('SELECT COUNT(id) FROM warnings WHERE guildid = $1', guildid)
                    memberid = await connection.fetchval('SELECT memberid FROM warnings WHERE guildid = $1', guildid)
                    member = guild.get_member(memberid)
                    mutedid = await mess
                    if count not in [3, 5, 7, 9, 10]:
                        continue
                    if(count == 3):
                        
                    
                    

    def __init__(self, bot: Bot) -> None:
        self.bot = bot
        self.raidmode = False
        self.settings: dict[str, int] = {}
        try:
            with open('automod.json', mode='r') as f:
                self.settings = json.load(f)
        except:
            pass
        self.cd_mapping: CooldownMapping = CooldownMapping.from_cooldown(10, 10, BucketType.member)
        self.autopunishment.start()

    @Cog.listener()
    async def on_member_join(self, member: Member):
        if self.raidmode:
            try:
                await member.send(
                    'We\'re expierencing a raid. Server has anti raidmode enabled. We kick any new members until the raid is over, please wait until the raid is over and try joining again later.')
            except Forbidden:
                pass
            await member.kick(reason='Anti-raid mode.')

    @command(aliases=['raidmode'])
    @has_any_role('Owner', 'Head Admin', 'Admin')
    async def antiraidmode(self, ctx: Context, status: str):
        if ctx.channel.guild is None:
            return
        if status.lower() in ['on', 'true']:
            if self.raidmode:
                await ctx.send('Anti-raid mode is already ON.')
            else:
                self.raidmode = True
                await ctx.send('Anti-raid mode ON. New members will be prevented from joining.')
        elif status.lower() in ['off', 'false']:
            if not self.raidmode:
                await ctx.send('Anti-raid mode is already OFF.')
            else:
                self.raidmode = False
                await ctx.send('Anti-raid mode OFF. New members are allowed to join.')
        else:
            await ctx.send('It\'s either ON, TRUE, FALSE or OFF. No other options.')

    @command()
    @has_any_role('Owner', 'Head Admin', 'Admin')
    async def anticopypasta(self, ctx: Context, arg: int):
        self.settings['copypasta'] = arg
        await ctx.send(f'Anti-copypasta settings updated! Users will now recieve {arg} warn(s) for each copypasta.')

    @command()
    @has_any_role('Owner', 'Head Admin', 'Admin')
    async def antiinvite(self, ctx: Context, arg: int):
        self.settings['invite'] = arg
        with open('automod.json', mode='w') as f:
            f.write(json.dumps(self.settings))
        await ctx.send(f'Anti-invite settings updated! Users will now receive {arg} warn(s) for each invite link.')

    @command()
    @has_any_role('Owner', 'Head Admin', 'Admin')
    async def antieveryone(self, ctx: Context, arg: int):
        self.settings['everyone'] = arg
        with open('automod.json', mode='w') as f:
            f.write(json.dumps(self.settings))
        await ctx.send(
            f'Anti-everyone settings updated! Users will now receive {arg} warn(s) for each everyone/here mention.')

    @command()
    @has_any_role('Owner', 'Head Admin', 'Admin')
    async def antispam(self, ctx: Context, arg: int):
        self.settings['spam'] = arg
        with open('automod.json', mode='w') as f:
            f.write(json.dumps(self.settings))
        await ctx.send(f'Anti-spam settings updated! Members will now receive {arg} warns for each spam message.')

    @command()
    @has_any_role('Owner', 'Head Admin', 'Admin')
    async def antimention(self, ctx: Context, threshold: int):
        self.settings['mention'] = threshold
        with open('automod.json', mode='w') as f:
            f.write(json.dumps(self.settings))
        await ctx.send(
            f'Anti-mass mention settings updated! Members will now receive a warn for each {threshold} mentions in '
            'message.')

    @command()
    @has_any_role('Owner', 'Head Admin', 'Admin')
    async def antireferral(self, ctx: Context, arg: int):
        self.settings['referral'] = arg
        with open('automod.json', mode='w') as f:
            f.write(json.dumps(self.settings))
        await ctx.send(f'Anti-referral settings updated! Members will now receive {arg} warns for each referral link.')

    @Cog.listener('on_message')
    async def nocommentonmedias(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        if message.guild.get_role(814388827396243516) in message.author.roles:
            return
        selfies = self.bot.channels['selfies']
        media = self.bot.channels['media']
        memes = self.bot.channels['memes']
        gamememes = self.bot.channels['game-memes']
        artroom = self.bot.channels['art-room']

        image_only = [selfies, media, memes, artroom, gamememes]

        if message.channel.id in image_only and not message.attachments:
            await message.delete()
            return

    @Cog.listener('on_message')
    async def spams(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        if message.guild.get_role(814388827396243516) in message.author.roles:
            return
        member: Member = message.guild.get_member(message.author.id)
        if message.channel.category.id == 830303275386798110:
            return
        warn: Command = self.bot.get_command('warn')
        bucket: CooldownMapping = self.cd_mapping.get_bucket(message)
        ratelimit = bucket.update_rate_limit()
        if ratelimit:
            warn: Command = self.bot.get_command('warn')
            for _ in range(1, self.settings['spam']):
                await warn(ctx=self.bot.get_context(message), member=message.author, reason='Spamming.')
            await message.delete()

    @Cog.listener('on_message')
    async def curses(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        member: Member = message.guild.get_member(message.author.id)
        warn: Command = self.bot.get_command('warn')
        censored: list[str] = ['nigger', 'nigga', 'n1gger', 'n199er', 'ni993r', 'ni88er', 'nigg3r', 'n1gga', 'n199a',
                               'nigg@', 'n199@', 'niger', 'n19er',
                               'n1gg@', 'retard', 'retord', 'niggas', 'niggers', 'n1ggers', 'n1ggas', 'ret@rd',
                               'reterd', 'returd', 'n!gga', 'n!gger', 'n!99a', 'n!993r']
        for curse in censored:
            if curse in message.content.lower():
                for _ in range(1, self.settings['spam']):
                    await warn(ctx=self.bot.get_context(message), member=message.author,
                               reason='Usage of censored word(s).')
                await message.delete()
                break

    @Cog.listener('on_message')
    async def hereeveryone(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        staff: Role = message.guild.get_role(814388827396243516)
        if staff in message.author.roles:
            return
        warn: Command = self.bot.get_command('warn')
        if '@here' in message.content or '@everyone' in message.content:
            for _ in range(1, self.settings['everyone']):
                await warn(ctx=self.bot.get_context(message), member=message.author, reason='Usage of censored word.')
            await message.delete()
            return

    @Cog.listener('on_message')
    async def copypasta(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        member: Member = message.guild.get_member(message.author.id)
        warn: Command = self.bot.get_command('warn')
        copypastas: list[str] = ['HOLD CTRL AND TYPE', 'only real WEEB can build this', 'FLIP THAT TABLE',
                                 '                                             ',
                                 '▬▬▬▬▬▬▬▬▬▬ஜ۩۞۩ஜ▬▬▬▬▬▬▬▬▬ haHAA ariW ariW haHAA &&& ▬▬▬▬▬▬▬▬▬▬ஜ۩۞۩ஜ▬▬▬▬▬▬▬▬▬', '4Head',
                                 'The FitnessGram™ Pacer Test &&& multistage aerobic capacity test that progressively gets more difficult as it continues.',
                                 'Check out these 4 cans', ' ▬ι══════ﺤ', 'only real LULer can build this', '☐ Not REKT',
                                 '☑ REKT ☑', 'REKTangle ☑', 'SHREKT ☑',
                                 'Total REKTall ☑ &&& The Lord of the REKT ☑ &&& The Usual SusREKTs ☑ &&& North by '
                                 'NorthREKT &&& ☑ REKT to the Future &&& ☑ Once Upon a Time in the REKT ☑ &&& The '
                                 'Good, the Bad, and the REKT ☑',
                                 'now playing: Who asked',
                                 'I sexually Identify as an overused sexually identification copypasta.',
                                 'Only the chosen one can stack these cans!',
                                 'Writing\'s not easy. That\'s why Grammarly can help &&& This sentence is '
                                 'grammatically correct, but it\'s wordy, and hard to read.',
                                 'Grammarly\'s cutting edge technology &&& helps you craft compelling, understandable '
                                 'writing that makes an impact on your reader.',
                                 'Yoshikage Kira', 'remove this part of the message after pasteing in chat',
                                 'was ejected', ' Anybody know what shungite is?',
                                 'I sexually Identify as an Attack Helicopter.', 'Paid viewer from the &&& Developers',
                                 'If you ask Rick Astley for a DVD of the movie Up, he won’t give it to you &&& he’s '
                                 'never gonna give you Up.',
                                 'Astley paradox',
                                 'To be fair, you have to have a very high IQ to understand Rick and Morty.',
                                 'IF YOU TOUCH THE DORITOS U ARE FAT',
                                 'CTRL+C CTRL+V ENTER',
                                 'When was the last time you saw a meme with such influence and beauty',
                                 'Yes I’m 14 &&& I mean seriously I’m sick of being single &&& I’m that "crazy kid" '
                                 'Everyone hates',
                                 'Not gonna be active on Discord tonight &&& I\'m meeting a girl (a real one) in half '
                                 'an hour &&& (wouldn\'t expect a lot of you to understand anyway)',
                                 'Not gonna 🔥 be active 🚬 on 🔛 Discord 😡 tonight 🌚. &&& I\'m 💘 meeting 💯 a '
                                 'girl 👩🍑 (a real 💯 one 😤) in half ➗ an hour 🕐 &&& (wouldn\'t 😩 expect 😀😃 a '
                                 'lot 🍑 of you 🤔👉 to understand 📚 anyway 🔛)',
                                 'Not gonna be active on Discowd tonight. &&& I\'m meeting a giww (a weaw one) in '
                                 'hawf an houw &&& (wouwdn\'t expect a wot of you to undewstand anyway)',
                                 'is going around sending friend requests to random Discord users &&& and those who '
                                 'accept his friend requests &&& accounts DDoSed and their groups exposed with the '
                                 'members inside &&& becoming a victim as well.',
                                 'copy and paste', 'False PSA',
                                 'out for a Discord &&& going around &&& those who accept &&& send this to as many '
                                 '&&& if you see this',
                                 'Fake Hacker PSA ',
                                 'not accept &&& friend request from &&& hacker &&& tell everyone && copy &&& paste',
                                 'Fake Discord shutdown',
                                 'discord &&& clos &&& populated &&& active &&& please send &&& copy &&& paste &&& '
                                 'deleted without hesitation',
                                 'Cooldog', '╰━▅╮ &&& ╰╮ &&& ┳━━╯ &&& ╰┳╯', '╰┳┳┳╯ &&& ▔╰━╯ &&& ╱╲╱╲▏', 'Memecat',
                                 'Λ＿Λ &&& ( \'ㅅ\' ) &&& >　⌒ヽ', 'Memecat (2)',
                                 'Λ＿Λ &&& ˇωˇ &&& >　⌒ヽ', 'Read or you die',
                                 'Carry on reading &&& Once there was &&& Now every week &&& send this to &&& copy '
                                 'and paste',
                                 'Jake Paul on a tower about to jump &&& copy and paste &&& discord server',
                                 'Tag you\'re it',
                                 'funny you opened this because &&& over the next &&& first you have &&& send it to '
                                 '&&& break the chain',
                                 '/▌ &&& /\ &&& This is bob', '╚═(███)═╝ &&& Lenn',
                                 '▐▄█▀▒▒▒▒▄▀█▄ &&& ▐▄█▄█▌▄▒▀▒ &&& ▒▀▀▄▄▒▒▒▄▒',
                                 'say about me &&& ll have you know I &&& I am trained &&& Think again &&& Not only '
                                 'am I extensively trained &&& kid',
                                 'have to have a very high IQ &&& extremely && solid grasp &&& deftly woven &&& '
                                 'heavily &&& intellectual capacity &&& cryptic reference',
                                 'years old &&& is love &&& is life &&& spread &&& push against &&& straight in the',
                                 'lⅰbra', 'libra-sale', 'linkairdrop.'
                                 ]
        for copypasta in copypastas:
            keywords = copypasta.split('&&&')
            count = 0
            for keyword in keywords:
                if keyword in message.content:
                    count += 1
                percentage = count / len(keyword) * 100 if len(keyword) > 0 else 0
                if percentage > 70:
                    for _ in range(1, self.settings['copypasta']):
                        await warn(ctx=self.bot.get_context(message), member=message.author,
                                   reason='Sending copypasta.')
                    await message.delete()
                    return
                count = 0
                match = SequenceMatcher(message.content, copypasta).ratio() * 100
                if match > 70:
                    for _ in range(1, self.settings['copypasta']):
                        await warn(ctx=self.bot.get_context(message), member=message.author,
                                   reason='Sending copypasta.')
                    await message.delete()
                    return

    @Cog.listener('on_message')
    async def referral(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        referrals = [
            '2no.co',
            'blasze.com',
            'blasze.tk',
            'gotyouripboi.com',
            'iplogger.com',
            'iplogger.org',
            'iplogger.ru',
            'ps3cfw.com',
            'yip.su',
            'bmwforum.co',
            'bucks.as',
            'cyberh1.xyz',
            'discörd.com',
            'disçordapp.com',
            'fortnight.space',
            'fortnitechat.site',
            'freegiftcards.co',
            'grabify.link',
            'joinmy.site',
            'leancoding.co',
            'minecräft.com'
            'quickmessage.us',
            'särahah.eu',
            'särahah.pl',
            'shört.co',
            'spötify.com',
            'spottyfly.com',
            'starbucks.bio',
            'starbucksisbadforyou.com',
            'starbucksiswrong.com',
            'stopify.co',
            'xda-developers.us',
            'youshouldclick.us',
            'yoütu.be',
            'yoütübe.co',
            'yoütübe.com',
            'youtubeshort.watch',
            'adblade.com',
            'adcash.com',
            'adcell.de',
            'adexchangecloud.com',
            'adf.ly',
            'adfoc.us',
            'adforce.com',
            'bc.vc',
            'bitl.cc',
            'btcclicks.com',
            'ceesty.com',
            'cur.lv',
            'fastclick.com',
            'getcryptotab.com',
            'gmads.net',
            'l2s.pet',
            'linkbucks.com',
            'linkshrink.net',
            'miniurl.pw',
            'nitroclicks.com',
            'ouo.io',
            'pay-ads.com',
            'petty.link',
            'pnd.tl',
            'restorecosm.bid',
            'sh.st',
            'short.es',
            'shorte.st',
            'shrtz.me',
            'udmoney.club',
            'uii.io',
            'ur-l.me',
            'vivads.net',
            'xponsor.com',
            'zeusclicks.com',
            'zipansion.com',
            'black-friday.ga',
            'boost.ink',
            'easycommerce.cf',
            'featu.re',
            'free.gg',
            'justdoit.cards',
            'makeprogress.ga',
            'pointsprizes.com',
            'referralpay.co',
            'selly.gg',
            'shoppy.gg',
            'weeklyjob.online',
            'wn.nr',
            'nakedphotos.club',
            'privatepage.vip',
            'viewc.site',
            'baymack.com',
            'btconline.io',
            'btcpool.io',
            'freebitco.in',
            'minero.cc',
            'outbuck.com',
            'amazingsexdating.com',
            'easter-event.com',
            'ezrobux.gg',
            'fortnite.cards',
            'fortnite.events',
            'fortnite-christmas.com',
            'fortnite-gifts.com',
            'fortnite-giveaway.com',
            'fortnite-special.com',
            'fortnite-vbuck.com',
            'fortnite-vbucks.de',
            'fortnite-vbucks.net',
            'fortnitevb.com',
            'free-gg.com',
            'free-steam-code.com',
            'giveawaybot.pw',
            'libra-sale.io',
            'myetherermwallet.com',
            'oprewards.com',
            'rbxfree.com',
            'roblox-christmas.com',
            'robloxsummer.com',
            'steam-event.com',
            'steam-gift-codes.com',
            'steam-money.org',
            'steam-wallet-rewards.com',
            'steampromote.com',
            'steamquests.com',
            'steamreward.com',
            'steamspecial.com',
            'steamsummer.com',
            'streamcommunnitly.com',
            'whatsappx.com',
            'getlⅰbra.tech'
        ]
        warn: Command = self.bot.get_command('warn')
        urls = re.findall('''
        (([\w]+:)?//)?(([\d\w]|%[a-fA-f\d]{2,2})+(:([\d\w]|%[a-fA-f\d]{2,2})+)?@)?([\d\w][-\d\w]{0,253}[\d\w]\.)+[\w]{2,63}(:[\d]+)?(/([-+_~.\d\w]|%[a-fA-f\d]{2,2})*)*(\?(&?([-+_~.\d\w]|%[a-fA-f\d]{2,2})=?)*)?(#([-+_~.\d\w]|%[a-fA-f\d]{2,2})*)?
        ''', message.content)
        for url in urls:
            shorturl = await reveal_short_url(url)
            if 'discord.gg' in shorturl:
                for _ in range(1, self.settings['invite']):
                    await warn(ctx=self.bot.get_context(message), member=message.author, reason='Sending invite links.')
            else:
                for referral in referrals:
                    if referral in message.content:
                        for _ in range(1, self.settings['referral']):
                            await warn(ctx=self.bot.get_context(message), member=message.author,
                                       reason='Sending referral links.')
                        await message.delete()
                        break

    @Cog.listener()
    async def on_message(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        warn: Command = self.bot.get_command('warn')
        if message.channel.category.id == 830303275386798110 or message.channel.id == 812913559847305267 or message.channel.id == 812488484617977886 or message.channel.id == 828875733580382238:
            return
        if len(message.mentions) > self.settings['mention']:
            await warn(ctx=self.bot.get_context(message), member=message.author, reason='Mass mention.')
            return

    @Cog.listener('on_message')
    async def long_message(self, message: Message):
        await self.bot.wait_until_ready()
        if message.author.bot:
            return
        if message.webhook_id is not None:
            return
        if message.guild is None:
            return
        staff: Role = message.guild.get_role(814388827396243516)
        if staff in message.author.roles:
            return
        if message.channel.category.id == 830303275386798110 or message.channel.id == 812913559847305267 or message.channel.id == 812488484617977886 or message.channel.id == 828875733580382238 or message.channel.id == 818055701007826945 or message.channel.category.id == 812318078511087678:
            return
        warn: Command = self.bot.get_command('warn')
        lines = message.content.split('\n')
        if len(lines) > 15:
            await message.delete()
            await warn(ctx=self.bot.get_context(message), member=message.author, reason='Long message.')


def setup(bot: Bot):
    bot.add_cog(Automod(bot))

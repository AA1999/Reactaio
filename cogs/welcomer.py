from discord import Guild, Member, Forbidden
from discord.channel import TextChannel
from discord.colour import Colour
from discord.embeds import Embed
from discord.ext.commands.bot import Bot
from discord.ext.commands.cog import Cog
from datetime import datetime
from discord.role import Role


def __ordinal__(number: int):
    if number % 10 == 1 and number != 11:
        return 'st'
    elif number % 10 == 2 and number != 12:
        return 'nd'
    elif number % 10 == 3 and number != 13:
        return 'rd'
    else:
        return 'th'


def __nonbots__(guild: Guild):
    return sum(not member.bot for member in guild.members)


class Welcomer(Cog):

    def __init__(self, bot: Bot):
        self.bot: Bot = bot

    @Cog.listener()
    async def on_member_join(self, member: Member):
        await self.bot.wait_until_ready()
        rules: TextChannel = self.bot.get_channel(self.bot.channels['rules'])
        selfroles: TextChannel = self.bot.get_channel(self.bot.channels['self-roles'])
        profiles: TextChannel = self.bot.get_channel(self.bot.channels['profiles'])
        welcomer: Embed = Embed(color=0x00ff33)
        welcomer.set_footer(text='We hope you enjoy your stay here!', icon_url=member.guild.icon_url)
        welcomer.set_thumbnail(url=member.guild.icon_url)
        welcomer.set_image(url='https://i.imgur.com/AxCn3Ar.gif')
        nonbots = __nonbots__(member.guild)
        count = str(nonbots) + __ordinal__(nonbots)
        welcome: TextChannel = member.guild.get_channel(self.bot.channels['welcome'])
        welcomer.title = f'Welcome, {str(member)}!'
        welcomer.description = f'Welcome to {member.guild.name}! You\'re our {count} member. \n'
        f'please read the {rules.mention} and assign {selfroles.mention} to unlock the rest of the server.\n'
        f'Also don\'t forget making a {profiles.mention}.'
        welcomer.set_author(name=str(member), icon_url=member.avatar_url)
        await welcome.send(embed=welcomer, content=f'Hey, {member.mention}!')
        if not member.bot:
            try:
                await member.send(
                    f'Enjoy your time in {rules.guild.name}.\nBut before that, please read the {rules.mention} '
                    f'and indefinitely assign {selfroles.mention}. Also create a profile in {profiles.mention}')
            except Forbidden:
                pass
        welcomers: Role = member.guild.get_role(830710331709063168)
        general: TextChannel = member.guild.get_channel(self.bot.channels['general'])
        alert: Embed = Embed(title=f"{count} Member", color=Colour.random(), timestamp=datetime.utcnow())
        alert.description = f'{member.mention} has joined {member.guild.name}. We welcome them and hope they enjoy their time here and find whatever they seek for.' 
        f'Please assign {selfroles.mention} and head to {profiles.mention} to create a profile.'
        alert.set_footer(text=f'{member.display_name}#{member.discriminator}', icon_url=member.avatar_url)
        alert.set_image(url='https://i.imgur.com/CgPnjfk.jpeg')
        alert.set_author(name=f'{self.bot.user.display_name}', icon_url=self.bot.user.avatar_url)
        await general.send(f'{welcomers.mention}, ', embed=alert)
        

def setup(client: Bot):
    client.add_cog(Welcomer(client))

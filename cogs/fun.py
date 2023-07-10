import random
from datetime import datetime

from discord import embeds
from discord.embeds import Embed
from discord.ext import commands
from discord.ext.commands import command
from discord.ext.commands.bot import Bot
from discord.ext.commands.cog import Cog


class Fun(Cog):

    def __init__(self, bot: Bot) -> None:
        self.bot: Bot = bot
        
    @command(name='8ball')
    async def _8ball(self, ctx, *, question):
        responses = ['As I see it, yes.',
        'Ask again later.',
        'Better not tell you now.',
        'Cannot predict now.',
        'Concentrate and ask again.',
        'Don’t count on it.',
        'It is certain.',
        'It is decidedly so.',
        'Most likely.',
        'My reply is no.',
        'My sources say no.',
        'Outlook not so good.',
        'Outlook good.',
        'Reply hazy, try again.',
        'Signs point to yes.',
        'Very doubtful.',
        'Without a doubt.',
        'Yes.',
        'Yes – definitely.',
        'You may rely on it.'
        ]
        ball: Embed = Embed(title = '8 Ball', color = 0x000000, timestamp = datetime.utcnow())
        ball.set_image(url = 'https://cdn.britannica.com/82/191982-050-1DF10DB5/ball.jpg')
        ball.add_field(name = 'Question: ', value = question, inline = False)
        ball.add_field(name = 'Answer: ', value = random.choice(responses), inline = False)
        await ctx.send(embed = ball)


def setup(bot: Bot):
    bot.add_cog(Fun(bot))

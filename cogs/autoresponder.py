from discord import Message, Role
from discord.ext.commands import Cog, Bot, Command


class AutoResponder(Cog):
    bot: Bot

    def __init__(self, bot: Bot):
        self.bot = bot

    @Cog.listener()
    async def on_message(self, message: Message):
        await self.bot.wait_until_ready()
        if message.channel.id == self.bot.channels['partnerships'] and not message.author.bot:
            partnerships: Role = message.guild.get_role(818409697963016202)
            await message.channel.send(partnerships.mention)
            return
        elif message.channel.id == self.bot.channels['partnerships'] and message.author.bot:
            return
        if message.content.lower() == 'rules' and not message.author.bot:
            post: Command = self.bot.get_command('embed post')
            await post(ctx=await self.bot.get_context(message), name='rules', channel=message.channel)


def setup(bot: Bot):
    bot.add_cog(AutoResponder(bot))

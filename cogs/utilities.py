from discord.channel import CategoryChannel, TextChannel
from discord.ext.commands import Cog, Bot, Context
from discord.ext.commands.core import command, has_guild_permissions, has_role
from discord.ext.commands.help import Paginator
from discord.guild import Guild


class Utilities(Cog):
    bot: Bot

    def __init__(self, bot: Bot):
        self.bot = bot

    @has_guild_permissions(manage_channels=True)
    @command(aliases=['mk'])
    async def makechannel(self, ctx: Context, category: str, *channels):
        guild: Guild = ctx.guild
        cat: CategoryChannel = await guild.create_category_channel(name=category, reason=f'By the request of {ctx.author.display_name}')
        for channel in channels:
            await guild.create_text_channel(name=channel, category=cat, reason=f'Requested by {ctx.author.display_name}')
        await ctx.send('Channels successfully created.')
    
    @command(name='list')
    @has_role('Owner')
    async def __list(self, ctx: Context):
        guild: Guild = ctx.guild
        channel: TextChannel
        result: dict[str, int] = {}
        for channel in guild.text_channels:
            result[channel.name] = channel.id
        paginator: Paginator = Paginator(prefix='', suffix='')
        paginator.add_line('List of server text channels: \n\n')
        for key in result:
            paginator.add_line(f'{key}: {result[key]}')
        for page in paginator.pages:
            await ctx.send(page)
    

def setup(bot: Bot):
    bot.add_cog(Utilities(bot))
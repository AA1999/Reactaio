from pathlib import Path

from discord import Role, Guild
from discord.ext.commands import Cog, Bot, has_permissions, command, has_role, Context, Paginator, \
    ExtensionAlreadyLoaded
from discord.utils import get


class Core(Cog):
    bot: Bot

    def __init__(self, bot: Bot):
        self.bot = bot

    @command()
    @has_permissions(mention_everyone=True)
    async def revive(self, ctx: Context):
        await ctx.send(f'**{ctx.author.display_name}#{ctx.author.discriminator}** has used revive command!')
        chat_revive: Role = get(ctx.guild.roles, id=813018522082607125)
        await ctx.send(f'{chat_revive.mention} Get active!')

    @command()
    @has_permissions(administrator=True)
    async def load(self, ctx: Context, extension):
        self.bot.load_extension(f'cogs.{extension}')

    @command()
    @has_role('Owner')
    async def shutdown(self, ctx: Context):
        await ctx.send('Shutting down...')
        await self.bot.close()

    @command()
    @has_permissions(administrator=True)
    async def unload(self, ctx: Context, extension):
        self.bot.unload_extension(f'cogs.{extension}')

    @command(aliases=['reload'])
    @has_permissions(administrator=True)
    async def reload_extension(self, ctx: Context, extension):
        try:
            await self.bot.reload_extension(f'cogs.{extension}')
            await ctx.send(f'Successfully reloaded {extension} Cog!')
        except:
            await ctx.send(f'{extension} did not load. Try reloading it again.')

    @command(name='reloadall')
    @has_permissions(administrator=True)
    async def reload_all(self, ctx: Context):
        for fileName in Path('cogs').glob('**/*.py'):
            *tree, _ = fileName.parts
            try:
                self.bot.load_extension(f"{'.'.join(tree)}.{fileName.stem}")
            except ExtensionAlreadyLoaded:
                self.bot.reload_extension(f"{'.'.join(tree)}.{fileName.stem}")
        await ctx.send('Successfully reloaded all cogs.')

    @command()
    @has_permissions(manage_roles=True)
    async def lsroles(self, ctx: Context):
        guild: Guild = ctx.guild
        paginator: Paginator = Paginator(prefix='', suffix='')
        roles: list[Role] = guild.roles
        roles.reverse()
        roles.remove(guild.default_role)
        for role in roles:
            paginator.add_line(f'**{role.name}** ({role.id})')
        for page in paginator.pages:
            await ctx.send(page)


def setup(bot: Bot):
    bot.add_cog(Core(bot))

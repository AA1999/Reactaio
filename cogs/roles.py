from typing import Union

from discord import Role, Guild
from discord.ext.commands import Paginator
from discord.ext.commands.bot import Bot, Cog, Context
from discord.ext.commands.core import command
from discord.member import Member


class Roles(Cog):

    async def get_role(self, role_name: Union[Role, str]):
        if isinstance(role_name, Role):
            return role_name
        guild: Guild = self.bot.get_guild(812314425318440961)
        roles: list[Role] = await guild.fetch_roles()
        roles.reverse()
        if isinstance(role_name, str):
            if role_name.startswith('<@&') and role_name.endswith('>'):
                for role in roles:
                    if role.mention.lower() == role_name.lower():
                        return role
            for role in roles:
                if role_name.lower() in role.name.lower():
                    return role
        return None

    def __init__(self, bot: Bot) -> None:
        self.bot: Bot = bot

    @command()
    async def role(self, ctx: Context, val: Union[Member, Role, str], *args):
        toremove: list[Role] = []
        toadd: list[Role] = []
        for arg in args:
            if arg.startswith('-'):
                role: Role = await self.get_role(arg[1:])
                if role is None:
                    await ctx.send('Invalid role(s). Please try again.')
                    return
                toremove.append(role)
            else:
                role: Role = await self.get_role(arg[1:]) if arg.startswith('+') else await  self.get_role(arg)
                if role is None:
                    await ctx.send('Invalid role(s). Please try again.')
                    return
                toadd.append(role)
        if isinstance(val, Role):
            count = sum(val in member.roles for member in ctx.guild.members)
            await ctx.send(f'Changing roles for {count} members. This will approximately take {count * len(args)} '
                           f'seconds.')

            for member in ctx.guild.members:
                roles: list[Role] = member.roles
                if val in roles:
                    for r in roles:
                        if r in toremove:
                            roles.remove(r)
                    roles += toadd
                    roles = list(set(roles))
                    await member.edit(roles=roles, reason=f'Changed by {ctx.author.display_name}')
            await ctx.send(f'Done changing roles for {count} members.')
        elif isinstance(val, Member):
            await ctx.send(f'Changing roles for **{val.display_name}**. This will approximately take {len(args)} '
                           f'seconds.')
            roles: list[Role] = val.roles
            for r in roles:
                if r in toremove:
                    roles.remove(r)
            roles += toadd
            roles = list(set(roles))
            await val.edit(roles=roles, reason=f'Changed by {ctx.author.display_name}')
            await ctx.send(f'Done changing roles for **{val.display_name}**.')
        elif isinstance(val, str):
            if val.lower() in ['humans', 'human', 'nonbots']:
                count = sum(not member.bot for member in ctx.guild.members)
                await ctx.send(f'Changing roles for {count} members. This will approximately take {count * len(args)} '
                               f'seconds.')
                for member in ctx.guild.members:
                    if not member.bot:
                        roles: list[Role] = member.roles
                        for r in roles:
                            if r in toremove:
                                roles.remove(r)
                        roles += toadd
                        roles = list(set(roles))
                        await member.edit(roles=roles, reason=f'Changed by {ctx.author.display_name}')
                await ctx.send(f'Done changing roles for {count} members.')
            elif val.lower() in ['bots', 'robots']:
                count = sum(1 for member in ctx.guild.members if member.bot)
                await ctx.send(f'Changing roles for {count} members. This will approximately take {count * len(args)} '
                               f'seconds.')
                for member in ctx.guild.members:
                    if member.bot:
                        roles: list[Role] = member.roles
                        for r in roles:
                            if r in toremove:
                                roles.remove(r)
                        roles += toadd
                        roles = list(set(roles))
                        await member.edit(roles=roles, reason=f'Changed by {ctx.author.display_name}')
                await ctx.send(f'Done changing roles for {count} members.')
            elif val.lower() in ['all', 'everyone']:
                count = ctx.guild.member_count
                await ctx.send(f'Changing roles for {count} members. This will approximately take {count * len(args)} '
                               f'seconds.')
                for member in ctx.guild.members:
                    roles: list[Role] = member.roles
                    for r in roles:
                        if r in toremove:
                            roles.remove(r)
                    roles += toadd
                    roles = list(set(roles))
                    await member.edit(roles=roles, reason=f'Changed by {ctx.author.display_name}')
                await ctx.send(f'Done changing roles for {count} members.')
            elif val.startswith('<@&') and val.endswith('>'):
                r: Role = self.get_role(val)
                count = sum(r in member.roles for member in ctx.guild.members)
                await ctx.send(f'Changing roles for {count} members. This will approximately take {count * len(args)} '
                               f'seconds.')
                for member in ctx.guild.members:
                    roles: list[Role] = member.roles
                    if r in roles:
                        for rle in roles:
                            if rle in toremove:
                                roles.remove(rle)
                        roles += toadd
                        roles = list(set(roles))
                        await member.edit(roles=roles, reason=f'Changed by {ctx.author.display_name}')

    @command()
    async def roles(self, ctx: Context, member: Member = None):
        if member is None:
            member = ctx.author
        paginator: Paginator = Paginator(prefix='', suffix='')
        roles: list[Role] = member.roles
        roles.reverse()
        roles.remove(ctx.guild.default_role)
        for role in roles:
            paginator.add_line(f'**{role.name}**\n')
        for page in paginator.pages:
            await ctx.send(page)


def setup(bot: Bot):
    bot.add_cog(Roles(bot))

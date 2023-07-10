import json
from datetime import datetime
from typing import Optional

import discord
from asyncpg.connection import Connection
from discord import Message
from discord.channel import TextChannel
from discord.colour import Colour
from discord.ext import commands
from discord.ext.commands.context import Context
from discord.ext.commands.converter import ColourConverter
from discord.ext.commands.core import group, has_any_role


class Embed(commands.Cog):
    bot: commands.Bot

    def __init__(self, bot: commands.Bot):
        self.bot = bot

    @group(invoke_without_command=True, case_insensitive=True)
    @has_any_role('Staff')
    async def embed(self, ctx: Context):
        if ctx.invoked_subcommand is None:
            info: discord.Embed = discord.Embed(title='Embed help', color=0x00ffcc, timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}embed [subcommand] [name/property]'
            info.add_field(name='Examples: ',
                           value=f'{self.bot.command_prefix}embed create <name> \n{self.bot.command_prefix}embed update <property> \n{self.bot.command_prefix}embed setchannel <channel>',
                           inline=False)
            await ctx.send(embed=info)

    @embed.command(aliases=['c'])
    async def create(self, ctx: Context, name: Optional[str]):
        if name is None:
            info: discord.Embed = discord.Embed(title='Embed help', color=0x00ffcc, timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}embed create [name]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}embed create coolembed', inline=False)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id = await connection.fetchval('SELECT name FROM embeds WHERE name = $1', (name), )
                if id is not None:
                    await ctx.send(
                        f'Embed already exists with the name **{id}**. Try a different name or use `{self.bot.command_prefix}embed update` ')
                else:
                    await connection.set_type_codec(
                        'json',
                        encoder=json.dumps,
                        decoder=json.loads,
                        schema='pg_catalog'
                    )
                    embed: discord.Embed = discord.Embed(title='New embed', color=Colour.random())
                    await ctx.send('Embed successfully created! Preview: ', embed=embed)
                    await connection.execute(
                        'INSERT INTO embeds (name, embed, datetimecreated) VALUES($1, $2::json, $3)', name,
                        embed.to_dict(), datetime.utcnow(), )

    @embed.command(aliases=['u', 'edit', 'e'])
    async def update(self, ctx: Context, name: Optional[str], property: Optional[str], *, newvalue: Optional[str]):
        if name is None:
            info: discord.Embed = discord.Embed(title='Embed help', color=0x00ffcc, timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}embed update [property] [new_value]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}embed update footertext Hello yall!',
                           inline=False)
            return
        if newvalue in ['server_name', 'server name', 'server-name']:
            newvalue = ctx.guild.name
        elif newvalue in ['server_icon', 'server icon', 'server-icon']:
            newvalue = ctx.guild.icon_url
        elif newvalue in ['server_banner', 'server banner', 'server-banner']:
            newvalue = ctx.guild.banner_url
        elif newvalue in ['pfp', 'author pfp', 'author_pfp', 'author-pfp', 'avatar']:
            newvalue = ctx.author.avatar_url
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.set_type_codec(
                    'json',
                    encoder=json.dumps,
                    decoder=json.loads,
                    schema='pg_catalog'
                )
                data = await connection.fetchval('SELECT embed::json FROM embeds WHERE name = $1', (name), )
                if data is None:
                    await ctx.send(f'Embed with name **{name}** does not exist. Try using a different name.')
                else:

                    embed: discord.Embed = discord.Embed.from_dict(data)
                    if property == 'color':
                        if embed.description is None:
                            embed.description = '⠀'
                        color = ColourConverter()
                        colour = await color.convert(ctx, newvalue)
                        embed.colour = colour
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property == 'authortext':
                        if embed.description is None:
                            embed.description = '⠀'
                        image = embed.author.url
                        embed.set_author(name=newvalue, url=image)
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property == 'authorimg':
                        if embed.description is None:
                            embed.description = '⠀'
                        if newvalue is None or newvalue == ' ' or newvalue.lower() == 'none':
                            author = None
                        else:
                            author = embed.author.name
                        embed.set_author(name=author, url=newvalue)
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property == 'title':
                        embed.title = newvalue
                        if embed.description is None:
                            embed.description = '⠀'
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property == 'thumbnail':
                        if embed.description is None:
                            embed.description = '⠀'
                        if newvalue is None or newvalue == ' ' or newvalue.lower() == 'none':
                            embed.set_thumbnail(url=discord.Embed.Empty)
                        else:
                            embed.set_thumbnail(url=newvalue)
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property == 'description':
                        if embed.description is None:
                            embed.description = '⠀'
                        embed.description = newvalue
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property == 'image':
                        if embed.description is None:
                            embed.description = '⠀'
                        embed.set_image(url=newvalue)
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property == 'footerimg':
                        if embed.description is None:
                            embed.description = '⠀'
                        footer = embed.footer.text
                        if newvalue is None or newvalue == ' ':
                            embed.set_footer(text=footer, icon_url=discord.Embed.Empty)
                        else:
                            embed.set_footer(text=footer, icon_url=newvalue)
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    elif property in ['footertext', 'footer']:
                        if embed.description is None:
                            embed.description = '⠀'
                        image = embed.footer.icon_url
                        embed.set_footer(text=newvalue, icon_url=image)
                        await ctx.send('Embed successfully updated! Preview: ', embed=embed)
                        await connection.execute('UPDATE embeds SET embed = $1::json WHERE name = $2', embed.to_dict(),
                                                 name, )
                    else:
                        await ctx.send(
                            'Invalid property. Properties are **authortext** ,**color**, **authorimg**, **title**, **description**, **thumbnail**, **image**, **footerimg**, **footertext** (**footer**).')

    @embed.command(aliases=['r', 'rem', 'del', 'delete', 'd'])
    async def remove(self, ctx: Context, name: Optional[str]):
        if name is None:
            info: discord.Embed = discord.Embed(title='Embed help', color=0x00ffcc, timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}embed remove [name]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}embed remove coolembed', inline=False)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                id = await connection.fetchval('SELECT name FROM embeds WHERE name = $1', (name), )
                if id is None:
                    await ctx.send(f'Embed does not exists with the name **{id}**. Try a different name.')
                else:
                    await ctx.send('Embed successfully deleted!')
                    await connection.execute('DELETE FROM embeds WHERE name = $1', (name), )

    @embed.command(aliases=['p'])
    async def post(self, ctx: Context, name: Optional[str], channel: Optional[TextChannel]):
        if name is None:
            info: discord.Embed = discord.Embed(title='Embed help', color=0x00ffcc, timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}embed post [name] [channel]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}embed post coolembed #cool-channel',
                           inline=False)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.set_type_codec(
                    'json',
                    encoder=json.dumps,
                    decoder=json.loads,
                    schema='pg_catalog'
                )
                data = await connection.fetchval('SELECT embed::json FROM embeds WHERE name = $1', (name))
                if data is None:
                    await ctx.send(
                        f'Embed **{name}** does not exist. Please try a different name or create one with that name.')
                else:
                    embed: discord.Embed = discord.Embed.from_dict(data)
                    await channel.send(embed=embed)
                    await ctx.send(f'Embed **{name}** has been successfully posted. Check {channel.mention}')

    @embed.command(aliases=['ls'])
    async def list(self, ctx: Context):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                embeds = await connection.fetch('SELECT name FROM embeds')
                if embeds is None:
                    await ctx.send('No embeds for this server made by Reactaio.')
                else:
                    result: discord.Embed = discord.Embed(title=f'{ctx.guild.name}\'s Embeds: ', color=0x0000ff,
                                                          timestamp=datetime.utcnow())
                    result.set_footer(text=ctx.guild.name, icon_url=ctx.guild.icon_url)
                    for id, embed in enumerate(embeds, start=1):
                        result.add_field(name=f'Embed #{id}: ', value=embed, inline=False)
                    await ctx.send(embed=result)

    @embed.command(aliases=['up'])
    async def updatepost(self, ctx: Context, name: Optional[str], message: Message):
        if name is None:
            info: discord.Embed = discord.Embed(title='Embed help', color=0x00ffcc, timestamp=datetime.utcnow())
            info.description = f'{self.bot.command_prefix}embed updatepost [name] [message]'
            info.add_field(name='Example: ', value=f'{self.bot.command_prefix}embed updatepost coolembed 1234',
                           inline=False)
            return
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.set_type_codec(
                    'json',
                    encoder=json.dumps,
                    decoder=json.loads,
                    schema='pg_catalog'
                )
                data = await connection.fetchval('SELECT embed::json FROM embeds WHERE name = $1', (name))
                if data is None:
                    await ctx.send(
                        f'Embed **{name}** does not exist. Please try a different name or create one with that name.')
                else:
                    embed: discord.Embed = discord.Embed.from_dict(data)
                    await message.edit(embed=embed)
                    await ctx.send(f'Posted embed **{name}** successfully updated! Please view the result at '
                                   f'{message.jump_url}.')


def setup(bot):
    bot.add_cog(Embed(bot))

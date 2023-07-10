import asyncio
import datetime
import random

import discord
from discord.ext import commands
from discord.ext.commands import Bot, Cog, Context


class Giveaway(Cog):

    def __init__(self, bot: Bot):
        self.bot: Bot = bot

    @commands.command(aliases=['gstart'])
    @commands.has_role('Owner')
    async def giveawaystart(self, ctx: Context, time: str, *, prize: str):
        seconds: int = 0
        if time.endswith('y'):
            seconds = int(time[:-1]) * 365 * 24 * 60 * 60
        elif time.endswith('d'):
            seconds = int(time[:-1]) * 24 * 60 * 60
        elif time.endswith('h'):
            seconds = int(time[:-1]) * 60 * 60
        elif time.endswith('m'):
            seconds = int(time[:-1]) * 60
        elif time.endswith('s'):
            seconds = int(time[:-1])
        embed: discord.Embed = discord.Embed(title = f'{prize} giveaway', color = 0x00ffdd)
        embed.add_field(name = 'Starts in:',value = str(datetime.datetime.utcnow()), inline = False)
        embed.add_field(name = 'Ends in:', value = str(datetime.datetime.utcnow() + datetime.timedelta(seconds = seconds)), inline = False)
        embed.set_footer(text='React with the 🎉 emoji to access the giveaway.')
        message = await ctx.send(embed = embed)

        await message.add_reaction('🎉')

        await asyncio.sleep(delay = seconds)

        newmsg: discord.Member = await ctx.channel.fetch_message(message.id)

        users = newmsg.reactions[0].users().flatten()
        users.pop(users.index(self.bot.user))
        users.pop(ctx.author)

        winner: discord.Member = random.choice(users)

        win: discord.Embed = discord.Embed(title = 'Winner', color = 0xff0000)
        win.set_footer(text=f'Best of wishes to them. DM **{ctx.author.display_name}#{ctx.author.discriminator}** to claim their prize!')
        win.description = f'Congratulations, {winner.mention}! You have won {prize}!'
        win.set_author(name='Giveaway bot')

        await ctx.send(embed = win)


def setup(bot: Bot):
    bot.add_cog(Giveaway(bot))

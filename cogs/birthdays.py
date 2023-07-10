import datetime
from datetime import date, datetime
import pytz
from asyncpg import Connection
from discord import TextChannel, Role
from discord.ext.commands import Bot, Cog, command, Context, Paginator
from discord.ext.tasks import loop
from pytz import UnknownTimeZoneError


class Birthdays(Cog):
    bot: Bot

    @loop(hours=1)
    async def birthdaycheck(self):
        await self.bot.wait_until_ready()
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                birthdays: TextChannel = self.bot.get_channel(self.bot.channels['🎂-birthdays'])
                birthdaysping: Role = birthdays.guild.get_role(822155551860719616)
                bdays = await connection.fetch('SELECT memberid, bday FROM bdays')
                for bday in bdays:
                    if bday['bday'].year == date.today().year and bday['bday'].month == date.today().month and \
                            bday['bday'].day == date.today().day and bday['bday'].hour == datetime.utcnow().hour:
                        await birthdays.send(f'{birthdaysping.mention}, it\'s '
                                             f'{self.bot.get_user(bday["memberid"]).mention}\'s birthday!'
                                             f'\nThey\'re now {date.today().year - bday["bday"].year} years old.')

    def __init__(self, bot: Bot):
        self.bot = bot
        self.birthdaycheck.start()

    @command()
    async def timezones(self, ctx: Context):
        paginator: Paginator = Paginator(prefix='', suffix='')
        await ctx.send('All timezones: ')
        timezones = list(pytz.all_timezones_set)
        timezones.sort()
        for tz in timezones:
            paginator.add_line(tz)
        for page in paginator.pages:
            await ctx.send(page)

    @command(aliases=['remember'])
    async def remember_birthday(self, ctx: Context, bday: str, timezone: str):
        if ctx.channel.id != self.bot.channels['reactaio']:
            return
        try:
            tz = pytz.timezone(timezone)
        except UnknownTimeZoneError:
            await ctx.send(f'Incorrect timezone, please view all available timezones using '
                           f'**{self.bot.command_prefix}timezones**.')
            return
        try:
            day: date = datetime.strptime(bday, '%Y-%m-%d')
            day: datetime = datetime(day.year, day.month, day.day, 15, 30, 0, 0, tz)
        except:
            await ctx.send('Invalid date. Date should be in format **YYYY-MM-DD**. Example 2000/01/01')
            return

        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                await connection.execute('INSERT INTO bdays (memberid, bday) VALUES($1, $2) ON CONFLICT(memberid)'
                                         'DO UPDATE SET bday = $2', ctx.author.id, day, )
                await ctx.send(f'Birthday remembered at {bday}!')


def setup(bot: Bot):
    bot.add_cog(Birthdays(bot))
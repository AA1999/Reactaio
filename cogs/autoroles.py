from datetime import datetime
from asyncpg.connection import Connection
from discord import Member
from discord.ext.commands.bot import Bot
from discord.ext.commands.cog import Cog
from discord.role import Role


class Autoroles(Cog):
    bot: Bot

    def __init__(self, bot: Bot):
        self.bot = bot

    @Cog.listener()
    async def on_member_join(self, member: Member):
        await self.bot.wait_until_ready()
        if member.bot:
            bots: Role = member.guild.get_role(812373896455258173)
            await member.add_roles(bots, reason='Auto roles for bots.')
            return
        verification: Role = member.guild.get_role(812702753575010316)
        age: Role = member.guild.get_role(812320009682419712)
        gender: Role = member.guild.get_role(812683341475348480)
        sexuality: Role = member.guild.get_role(812670976184811570)
        preferences: Role = member.guild.get_role(822146320365781044)
        mbti: Role = member.guild.get_role(813430648689655838)
        ethnicity: Role = member.guild.get_role(823925801925869568)
        zodiac: Role = member.guild.get_role(820018416558538793)
        hobbies: Role = member.guild.get_role(812320177340547143)
        height: Role = member.guild.get_role(859331559382056970)
        relationship: Role = member.guild.get_role(812658844223930388)
        location: Role = member.guild.get_role(813333491692994580)
        looking: Role = member.guild.get_role(812670114717499414)
        dm: Role = member.guild.get_role(812711598695120896)
        pings: Role = member.guild.get_role(813018292742520854)
        games: Role = member.guild.get_role(823921917064904775)
        bots: Role = member.guild.get_role(865187997301735454)

        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                ids = await connection.fetchval('SELECT roles FROM members WHERE memberid = $1', member.id, )
                if ids is not None:
                    for roleid in ids:
                        try:
                            await member.add_roles(member.guild.get_role(roleid), reason='Auto roles')
                        except:
                            pass
                else:
                    roles = [verification, age, gender, sexuality, preferences, mbti, ethnicity, zodiac, hobbies,
                             relationship, location, looking, dm, pings, games, height, bots]
                    await member.add_roles(*roles, reason='Auto-roles.')
                    ids: list[int] = []
                    role: Role
                    for role in member.roles:
                        ids.append(role.id)
                    await connection.execute(
                        'INSERT INTO members (memberid, roles, datetimejoined) VALUES ($1, $2, $3) '
                        'ON CONFLICT(memberid) DO UPDATE SET roles = $2',
                        member.id, ids, datetime.utcnow(), )
                    await connection.execute('INSERT INTO levels (memberid, textxp, textlevel, voicexp, voicelevel) '
                                             'VALUES ($1, 1, 1, 1, 1) ON CONFLICT(memberid) DO NOTHING ', member.id)

    @Cog.listener()
    async def on_member_update(self, before: Member, after: Member):
        connection: Connection
        async with self.bot.pool.acquire() as connection:
            async with connection.transaction():
                role_ids: list[int] = []
                for role in after.roles:
                    role_ids.append(role.id)
                await connection.execute(
                    'INSERT INTO members (memberid ,roles, datetimejoined) VALUES ($1, $2, $3) ON CONFLICT(memberid) '
                    'DO UPDATE SET roles = $2',
                    after.id, role_ids, after.joined_at, )
                await connection.execute('INSERT INTO levels (memberid, textxp, textlevel, voicexp, voicelevel) '
                                         'VALUES ($1, 0, 1, 0, 1) ON CONFLICT(memberid) DO NOTHING', after.id)
                await connection.execute('UPDATE members SET roles = $1 WHERE memberid = $2', role_ids, after.id, )


def setup(bot: Bot):
    bot.add_cog(Autoroles(bot))

from re import M
from discord import Invite, Guild, Member, User, TextChannel
from discord.ext.commands import Cog, Bot


class InviteData:
    inviter: User
    count: int

    def __init__(self, inviter: User, count: int):
        self.inviter = inviter
        self.count = count


class InviteLogger(Cog):
    bot: Bot
    invites: list[Invite]

    async def config(self):
        await self.bot.wait_until_ready()
        guild: Guild = self.bot.get_guild(812314425318440961)
        self.invites = await guild.invites()

    def __init__(self, bot: Bot):
        self.bot = bot
        bot.loop.create_task(self.config())

    @Cog.listener()
    async def on_invite_create(self, invite: Invite):
        self.invites = await invite.guild.invites()

    @Cog.listener()
    async def on_invite_delete(self, invite: Invite):
        self.invites = await invite.guild.invites()

    async def __inviter__(self, member: Member):
        invites: list[Invite] = await member.guild.invites()
        inv: Invite
        invite: Invite
        for inv, invite in zip(self.invites, invites):
            if inv.uses < invite.uses:
                self.invites = invites
                return InviteData(invite.inviter, invite.uses)
        return None
   

    @Cog.listener()
    async def on_member_join(self, member: Member):
        await self.bot.wait_until_ready()
        data: InviteData = await self.__inviter__(member)
        user: User = data.inviter
        count: int = data.count
        invitelogs: TextChannel = member.guild.get_channel(824847474409472000)
        if user is None:
            await invitelogs.send(f'Invite for {member.mention} not found, maybe a temporary invite.')
        elif member.bot:
            await invitelogs.send(f'{member.mention} is a bot invited by **{user.display_name}** who now has '
                                  f'{count} invites.')
        else:
            await invitelogs.send(f'{member.mention} is invited by **{user.display_name}** who now has '
                                  f'{count} invites.')


def setup(bot: Bot):
    bot.add_cog(InviteLogger(bot))

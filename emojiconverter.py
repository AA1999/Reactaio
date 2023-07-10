
import discord.ext.commands
from discord.ext.commands import BadArgument, IDConverter, Context, PartialEmojiConverter
from emojis.db import get_emoji_by_code


class EmojiNotFound(BadArgument):
    def __init__(self, argument):
        self.argument = argument
        super().__init__(f'Emoji "{argument}" not found.')


class EmojiConverter(IDConverter):
    async def convert(self, ctx: Context, argument):
        try:
            ec: discord.ext.commands.EmojiConverter = discord.ext.commands.EmojiConverter()
            result = await ec.convert(ctx, argument)
            return result
        except BadArgument:
            try:
                pec: PartialEmojiConverter = PartialEmojiConverter()
                result = await pec.convert(ctx, argument)
                return result
            except BadArgument:
                result = get_emoji_by_code(argument)
                if result is None:
                    raise EmojiNotFound(argument)
                return result.emoji

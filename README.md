# hackerb0t
###Summary
A Twitch IRC Chatbot, written in C, without an IRC library.

100% of this program's creation was live streamed on my twitch channel (http://www.twitch.com/hackerc0w)

hackerb0t uses cJSON for JSON parsing (Copyright (c) 2009 Dave Gamble)

### Fair warning
Please note that this is mostly intended to be an educational  project. I started it to learn things about the C programming language, the winsock library and the IRC protocol (especially Twitch's modification of it).

If you do decide to (for whatever reason) use this bot in a "production" environment, please do not expect it to work 100% of the time. In fact, don't expect it to work at all. Needless to say, I'm not responsible for any damage done by this piece of software.

### Commands
* !ping
    * Writes "Pong" to the chat. Mainly for debugging purposes (i.e. testing if the bot is up and responding)
* !saymyname
    * Writes the user's name to the chat. Again, mainly for debugging (tests if all the parameter stuff gets passed through correctly)
* !title
    * Fetches the current channel's stream title from the Twitch API and displays it in chat.
* !die
    * Writes a suicide note to chat and kills itself.
* !suggest [suggestion]
    * Takes a string as an argument (can contain spaces) and appends that string to a file called "suggestions.txt", followed by a newline.
* !uptime
    * Fetches the start time of the current channel's stream from the Twitch API and displays how long the stream has been running for in a nice format.
* !botsnack
    * Writes "Mmmmmh, delicious Kreygasm" to chat (where _Kreygasm_ is a Twitch emote)
* !lurk [channel name]
    * Joins another Twitch streamer's chat and lurks there. Note that the channel name has to be supplied without the leading _#_ (e.g. _hackerc0w_).
* !unlurk [channel name]
    * Leaves a channel that was previously joined by !lurk
* !sendto [channel] [message]
    * Displays _message_ in _channel_'s Twitch chat. Note that this only works if the bot is currently lurking in this channel.
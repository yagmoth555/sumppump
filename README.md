# sumppump
Bot that detects spam / rudeness in comments and posts links to it to chatrooms for quick deletion. 

Idea based of <a href="https://github.com/Charcoal-SE/SmokeDetector">smoke-detector</a> and done as a personnal's project. 

I wanted to add comments moderation, but as smoke-detector is done in python, while my expertise is c/c++. I did that port for that reason.

To be used on <a href="https://stackexchange.com/">StackExchange</a>.

# How it work

- The bot read the StackExchange API to list new comments.
- The bot follow the comments and try to discover other(s).
- The bot dig the Hot Network Question to dig popular's question to find other comments.
- The bot save all user statistics.
- The bot follow the active question list to find other contents.

# What is saved

- User statistics.
- Auto flagged comments.
- Comment ID digged.
- Question's digged.

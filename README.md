## Project Overview
- a SQL command line database
- Using BTrees as the current underlying structure
- Arenas are used for memory management
- We are going to be trying to do Tiger-Style development and as such we should allocate all memory we might need at the beginning to avoid runtime sys calls to get more virtual memory

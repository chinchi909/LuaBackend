## Installing LuaBackend Hook

### Internal LuaBackend Hook [DLL]
- Download ``DBGHELP.zip`` from the Releases tab.
- Extract the contents of ``DBGHELP.zip`` to the directory Kingdom Hearts 1.5+2.5 is installed which should be `KH_1.5_2.5`.
- Place your scripts in a folder named `scripts/[gameid]` in the `KINGDOM HEARTS HD 1.5+2.5 ReMIX` folder in your Documents folder.
    - For Kingdom Hearts Final Mix the default folder will be `scripts/kh1`.
    - For Kingdom Hearts Re: Chain of Memories the default folder will be `scripts/recom`.
    - For Kingdom Hearts II Final Mix the default folder will be `scripts/kh2`.
    - For Kingdom Hearts Birth by Sleep Final Mix the default folder will be `scripts/bbs`.
- The installation is now finished. LuaBackend will be automatically started with the game and can be easily uninstalled
by simply removing the ``DBGHELP.dll``. To verify it is installed correctly, you can open the LuaBackend console using
the F2 key on the keyboard in game.

### Custom script locations

A configuration file can be used to customize the script location(s).

- Edit the file called `LuaBackend.toml` in the same folder as `DBGHELP.dll` (the game install folder).
  - To edit the script paths, modify the `scripts` value.
  - `path` is the game path for an entry.
  - `relative`, when true, will append `path` to the `KINGDOM HEARTS HD 1.5+2.5 ReMIX` folder and will be an absolute path otherwise.
- This file can contain location(s) in which to look for scripts for each game.
- If _any_ valid locations are listed for a game, _only_ those location(s) are searched for scripts.
  This allows you to completely override the default location if the default location is causing problems.
- If _no_ valid locations are listed for a game, the default location from above is searched for scripts.

Sample file:

```
[kh1]
scripts = [{ path = "D:\kh1-scripts", relative = false }]
base = "3A0606"
thread_struct = "22B7280"
exe = "KINGDOM HEARTS FINAL MIX.exe"

[kh2]
scripts = [
  { path = "scripts\kh2", relative = true },
  { path = "C:\Users\johndoe\bin\KH\PC\openkh\mod\luascript", relative = false },
]
base = "56454E"
thread_struct = "89E9A0"
exe = "KINGDOM HEARTS II FINAL MIX.exe"
```

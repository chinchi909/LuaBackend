## Installing LuaBackend Hook

### Internal LuaBackend Hook [DLL]
- Download ``DBGHELP.zip`` from the Releases tab.
- For Kingdom Hearts 1.5+2.5
  - Extract the contents of ``DBGHELP.zip`` to the directory Kingdom Hearts 1.5+2.5 is installed which is named `KH_1.5_2.5`.
  - Place your scripts in a folder named `scripts/[gameid]` in the `KINGDOM HEARTS HD 1.5+2.5 ReMIX` folder in your Documents folder.
      - For Kingdom Hearts Final Mix the default folder will be `scripts/kh1`.
      - For Kingdom Hearts Re: Chain of Memories the default folder will be `scripts/recom`.
      - For Kingdom Hearts II Final Mix the default folder will be `scripts/kh2`.
      - For Kingdom Hearts Birth by Sleep Final Mix the default folder will be `scripts/bbs`.
  - The installation is now finished. LuaBackend will be automatically started with the game and can be easily uninstalled
  by removing the ``DBGHELP.dll``. To verify it is installed correctly, you can open the LuaBackend console using
  the F2 key on the keyboard in game.
- For Kingdom Hearts 2.8
  - The steps for 2.8 are the similar, but the game's install directory is named `KH_2.8` and the default script path
  will be in the `KINGDOM HEARTS HD 2.8 Final Chapter Prologue` folder.
  - For Kingdom Hearts Dream Drop Distance HD the default scripts folder will be `scripts/kh3d`.

### Custom script locations

The configuration file can be modified to customize the script location(s).

- Edit the file called `LuaBackend.toml` in the same folder as `DBGHELP.dll` (the game install folder).
  - To edit the script paths, modify the `scripts` value, which is the list of script paths that will be used.
  - `path` is the scripts path for an entry.
  - `relative`, when true, make the `path` relative to the game's documents folder specified
  by `game_docs` and will be an absolute path otherwise.

Sample configuration:

```
[kh1]
scripts = [{ path = "D:\\kh1-scripts", relative = false }]
exe = "KINGDOM HEARTS FINAL MIX.exe"
game_docs = "KINGDOM HEARTS HD 1.5+2.5 ReMIX"

[kh2]
scripts = [
  { path = "scripts\\kh2", relative = true },
  { path = "C:\\Users\\johndoe\\bin\\KH\\PC\\openkh\\mod\\luascript", relative = false },
]
exe = "KINGDOM HEARTS II FINAL MIX.exe"
game_docs = "KINGDOM HEARTS HD 1.5+2.5 ReMIX"
```

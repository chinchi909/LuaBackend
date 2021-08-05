# LuaBackend Hook
![Windows Build](https://github.com/Sirius902/LuaBackend/workflows/CI/badge.svg)

## What is this?

This is a fork of TopazTK's [LuaBackend](https://github.com/TopazTK/LuaBackend) which focuses on
consistency and reliability for timing-critical scripts.

## Why use this over LuaBackend or LuaFrontend?

For most scripts there's no issue using LuaBackend or LuaFrontend but for scripts such as the 
Garden of Assemblage (GoA) Randomizer, which relies on running once before every frame in-game, it's
important this requirement is met. LuaBackend and LuaFrontend currently aren't syncronized with
the game's main loop, leading to unintended behavior in scripts like GoA which can range from
crashes to warps to incorrect locations. This fork however syncronizes the Lua scripts with the game
loop effectively eliminating these issues. However, scripts that rely on new features from LuaFrontend
are not supported and it is recommended to use LuaFrontend concurrently with LuaBackend Hook to achieve this.

## Support
For the time being, only supports the Global version of KINGDOM HEARTS II FINAL MIX on PC.

Installation instructions are in **INSTALL.md**.

## Why is the dll file named DINPUT8?
This is so that LuaBackend Hook can hook into the game without requiring an EXE patch, but still
be able to launch automatically with the game.

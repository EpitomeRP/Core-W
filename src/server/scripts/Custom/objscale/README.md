#Objscale [![Build Status](https://travis-ci.org/Rochet2/TrinityCore.svg?branch=objscale)](https://travis-ci.org/Rochet2/TrinityCore)

####About
Objscale allows scaling individual spawns of creatures and gameobjects.
You can do this change ingame with commands or in the `creature` and `gameobject` database tables.
Made for 3.3.5a.<br />
Source: http://rochet2.github.io/Objscale.html

####Installation

Available as:
- Direct merge: https://github.com/Rochet2/TrinityCore/tree/objscale
- Diff: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:3.3.5...objscale.diff
- Diff in github view: https://github.com/Rochet2/TrinityCore/compare/TrinityCore:3.3.5...objscale

Using direct merge:
- open git bash to source location
- do `git remote add rochet2 https://github.com/Rochet2/TrinityCore.git`
- do `git pull rochet2 objscale`
- use cmake and compile

Using diff:
- DO NOT COPY THE DIFF DIRECTLY! It causes apply to fail.
- download the diff by __right clicking__ the link and select __Save link as__
- place the downloaded `objscale.diff` to the source root folder
- open git bash to source location
- do `git apply objscale.diff`
- use cmake and compile

After compiling:
- Navigate to `\src\server\scripts\Custom\objscale\sql\`
- Run `world.sql` to your world database
- Run `auth.sql` to your auth database

####Usage
You can set the sizes in the `creature` and `gameobject` database tables in the world database by changing the `size` column.
In game you can change a spawned object's size by using `.gobject set scale #guid #scale` and spawned NPC sizes by selecting an NPC and using `.npc set scale #scale`
Using -1 has scale makes the gameobject and npc use the default scale from template.

####Bugs and Contact
Report issues and similar to https://rochet2.github.io/

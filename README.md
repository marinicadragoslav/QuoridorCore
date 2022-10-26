# QuoridorCore

## Build

```
mkdir build
cd build
cmake ..
make
```

## Start console app

```
cd build/export/bin/
./quoridor-cli
```

## Create a new plugin

A plugin implements the logic of a Quoridor player.

To add a new plugin one must create a new subfolder under [plugins](plugins/) and add a new class that extends [qcore::Player](qcore/include/Player.h) and overrides **doNextMove()** method.

At each step, a player can interrogate the current state of the board and must complete one of the allowed actions (move to another position or place a wall on the board).

Use [dummy_plugin](plugins/dummy_plugin) as example.
To make the plugin available to the game controller, **REGISTER_QUORIDOR_PLAYER()** must be called with the new player class as parameter ([PlayerRegistration.cpp](plugins/dummy_plugin/src/PlayerRegistration.cpp) can be reused for this purpose).

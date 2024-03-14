# Ace Combat 7 UEVR compatibility mod

### Download UEVR Profile [here](https://github.com/keton/ace-combat-uevr/releases/latest)

## Features
1. fixes cockpit camera so it works out of the box in most situations
1. adds 'unstuck cockpit instruments' button (left start/select button on Xbox controller) for those rare occurrences where automation fails.
1. Adds optional controls remap selectable in plugin overlay in UEVR menu. For remap any standard RC control mode can be selected (Mode 1-4).

## Building

1. Install Visual Studio 2022 with C++, Windows SDK and CMake support
1. Make sure you follow all instructions [here](https://github.com/praydog/UEVR/blob/master/COMPILING.md) and are able to build UEVR from source
1. Clone the repository
1. Run `build.cmd` or in VS Developer Shell type:
	```shell
	cmake -S . -B build ./build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
	cmake --build ./build --clean-first --config Release --target ace_combat_plugin
	```
1. copy resulting `ace_combat_plugin.dll` to your game profile `plugins` subfolder

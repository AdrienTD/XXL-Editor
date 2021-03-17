# XXL-Editor

Level editor for games based on ELB's Kal engine, such as the Asterix & Obelix XXL series, written in C++11

![Screenshot 1](docs/screenshot01.jpg)

The level editor is still in early development, thus some features might be missing or buggy, depending on the game used.

Games supported include:

* A&O XXL 1 (2003)
* A&O XXL 2: Mission Las Vegum (2005)
* Asterix at the Olympic Games (2007)
* A&O XXL 2 HD (2018)
* A&O XXL Romastered (2020)

Support for other games using the Kal engine such as Arthur and Spyro might be possible in the future but is not in top priority.

Stuff that can be (partially) viewed and modified in the editor include:

* Scene nodes
* Beacons (locations for bonuses and crates)
* Enemy squads and choreographies
* Ground collision
* Pathfinding graph
* Trigger shapes
* Events
* Textures
* Sounds
* Model import/export in Renderware DFF format
* Localized text for XXL1

Both PC and Console versions will work, however 3D models, textures and sounds are only supported on PC version for now.

Licensed under the MIT License
Only works on Windows (Vista and later).

To download the editor, go to the [Releases page](https://github.com/AdrienTD/XXL-Editor/releases).
Check the included readme file for usage information.

## Compiling Requirements
* Visual Studio 2017/2019 (other versions untested)
* [vcpkg](https://github.com/Microsoft/vcpkg) with the following installed packages: inih, sdl2, stb
* Open the VS solution file, and as long as [vcpkg integration](https://docs.microsoft.com/en-us/cpp/build/integrate-vcpkg) is applied, you should be able to compile.

## Thanks
* Thanks to S.P.Q.R. for some code addition.

## Libraries used:
* [Dear Imgui](https://github.com/ocornut/imgui)
* [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
* [inih](https://github.com/benhoyt/inih)
* [SDL 2](https://www.libsdl.org)
* [stb](https://github.com/nothings/stb)

![Screenshot 2](docs/screenshot02.jpg)
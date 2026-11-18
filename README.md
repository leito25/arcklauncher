# Arcade Portfolio

A planned cross-platform arcade collection with a launcher, standalone games,
and shared player statistics. The Qt launcher discovers games from metadata
and starts SFML 2 and SFML 3 circle samples in separate processes. The SFML 3
sample records playtime using the local desktop SDK. Statistics display and
web support are planned. Windows is the initial development target.

## Build on Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload,
CMake 3.22 or newer, Git, and Qt 6.5 or newer with the MSVC 2022 64-bit kit,
Qt Quick, and Qt Quick Controls. Use a Qt kit built for the same compiler and
architecture as the application. Install Qt using the official installer;
the setup script does not install software or need administrator rights.
CMake fetches pinned SFML 2.6.2, 3.0.2 and nlohmann/json sources on first configuration;
an internet connection is required. Neither SFML nor Qt is a Git submodule.

Clone the repository, then in PowerShell replace the Qt path with your
installation:

```powershell
git clone https://github.com/leito25/arcklauncher.git
cd arcklauncher
powershell -NoProfile -ExecutionPolicy Bypass -File .\setup.ps1 -Action Test -Configuration Debug -QtPath "C:\Qt\6.8.3\msvc2022_64"
.\Deployment\Debug\ArcadeLauncher.exe
```

The script checks prerequisites, builds the launcher and both games in
separate CMake build trees, runs tests, and deploys Qt's runtime. To build
without running tests, omit `-Action Test`. If `QT_ROOT_DIR` points to your
Qt kit, `-QtPath` can be omitted. The script also discovers a single matching
kit under `C:\Qt`; with multiple kits, specify the path explicitly.
The command sets the execution policy only for that PowerShell process; it
does not change the machine or user policy.

Select **Play** on either sample; closing the game returns to the launcher.
On normal exit, the SFML 3 sample records whole seconds in
`%LOCALAPPDATA%\ArcadePortfolio\profile.json`. The SFML 2 sample does not yet
use the SDK. If saving fails the sample exits with an error; the launcher
reports the nonzero exit code and the game's error details.
Both games also run independently from their folders in
`.\Deployment\Debug\games\`. The launcher scans `games` relative to its own executable, not the
current working directory. Each game folder needs a `meta.json` with a
nonempty `title` and an `executable` filename in that same folder. The sample
metadata is generated and copied to the deployment folder by CMake. Use
**Refresh games** to rescan after adding or fixing a game.

Build and test Release, or create a development ZIP:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\setup.ps1 -Action Test -Configuration Release -QtPath "C:\Qt\6.8.3\msvc2022_64"
powershell -NoProfile -ExecutionPolicy Bypass -File .\setup.ps1 -Action Package -Configuration Release -QtPath "C:\Qt\6.8.3\msvc2022_64"
```

The ZIP in `out\` is **not distribution-ready**. Before sharing it, verify
Qt, SFML and third-party licenses, matching source availability, and Windows
runtime dependencies. See the architecture's licensing section.

## VS Code and CI

Install the Microsoft C/C++ extension. Set `QT_ROOT_DIR` before starting
VS Code, or install a single kit under `C:\Qt`. Run **Arcade: Build Debug**,
**Build Release**, **Test Debug**, or **Package Release** from Run Task.
The Run and Debug panel has Debug Launcher and Run Release configurations.
GitHub Actions builds and tests both configurations on Windows.

## Work on one game

Open either `Source\Games\Template SFML3` or `Source\Games\TemplateSFML2`
as a **folder in a new VS Code window**. The C/C++ extension, Git, CMake and
Visual Studio 2022 C++ tools are required; Qt is not needed for a game-only
build. The game remains part of this repository and uses its shared CMake
files and, for SFML 3, the Arcade SDK.

Open **Run and Debug** with `Ctrl+Shift+D` and select **Game: Debug** or
**Game: Run Release**. Press `F5` to start with the debugger, or `Ctrl+F5`
to run without debugging. Each configuration runs its corresponding build
task first. These are launch configurations, not tasks.

Open the Command Palette with `Ctrl+Shift+P` and choose **Tasks: Run Task**
for **Game: Build Debug**, **Game: Build Release**, **Game: Test Debug**,
**Game: Package Release**, **Game: Deploy Debug**, or **Game: Deploy Release**.
Build tasks compile and stage the game; they do not start it.

Each game uses its own `build\Debug` and `build\Release` directories. Builds
stage the game under that build directory, not in the launcher. Deploy copies
the staged game to the matching `Deployment\<configuration>\games` folder;
refresh the launcher to discover it. Deploy overwrites matching files but does
not remove older files, and staging also retains files from previous builds.
Package tests the Release build and writes a game-only ZIP to `out\games`.
The SFML 3 template keeps the executable and package name `TemplateSFML`
for compatibility despite its `Template SFML3` source folder. Like the
launcher ZIP, each game ZIP is for development
only: check licenses and runtime dependencies before distribution.

## Create a game

1. Copy `Source\Games\Template SFML3` for SFML 3 or
   `Source\Games\TemplateSFML2` for SFML 2. Keep the upstream
   `#include <SFML/Graphics.hpp>` and use the selected version's API.
2. Rename the executable and deployment targets, deployment folder, metadata
   title, and test name throughout the copied CMake file. Use the same name
   for the executable and deployment folder. If copying the SFML 3 sample, give it a
   unique SDK game ID as well. Update the game name and executable paths in
   the copied `.vscode\tasks.json` and `.vscode\launch.json`. Replace the sample's `main.cpp` with your
   game. Keep assets in the source folder and add a CMake copy command to
   deploy them alongside the executable.
3. Keep the copy directly under `Source\Games`, so its references to shared
   repository files continue to work. It can be built independently. To
   include it in the full launcher build, add an SFML 3 game with
   `add_subdirectory` in the root CMake project. An
   SFML 2 copy is a separate CMake project: add configure, build and test
   commands to `setup.ps1`, using its own build directory and the same
   `ARCADE_DEPLOYMENT_DIR` as the other games.
4. Run the setup command with `-Action Test`, refresh the launcher, and test the game.
   Never link SFML 2 and 3 to the same executable.

The shared C ABI is declared in `Source\ArcadeSDK\ArcadeSDK.h`. Link a game
to `ArcadeSDK`, deploy its shared library beside the game, initialize with a
stable lowercase game ID, check each result, and shut down on exit. See the
architecture for profile format, limitations and the future web roadmap.

## Planning documents

- [Architecture and initial roadmap](arcade_portfolio_architecture.md)
- [Requirements and open decisions](docs/requirements.md)
- [Planning workflow](docs/workflow.md)
- [Implementation todos](docs/todos.md)

The architecture document is a starting proposal, not a finalized design.
In particular, storage, concurrency, platform support, and packaging still
need explicit decisions.

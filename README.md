# Arcade Portfolio

A planned cross-platform arcade collection with a launcher, standalone games,
and shared player statistics. The Qt launcher discovers games from metadata
and starts SFML 2 and SFML 3 circle samples in separate processes. The SFML 3
sample records playtime using the local desktop SDK, and the launcher displays
combined and per-game playtime. A separate web launcher includes browser
samples and local statistics.
Windows is the initial development target.

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
use the SDK. The launcher watches this profile and displays total playtime
across games and playtime on each game card; updates from the SDK are
reflected while the launcher is open. Times are shown in whole minutes; games
without a profile entry show **Not played yet**.
If saving fails the sample exits with an error; the launcher reports the
nonzero exit code and the game's error details.
Both games also run independently from their folders in
`.\Deployment\Debug\games\`. The launcher scans `games` relative to its own executable, not the
current working directory. Each game folder needs a `meta.json` with a unique
lowercase `id`, a nonempty `title`, and an `executable` filename in that same
folder. IDs may contain lowercase ASCII letters, digits, `.`, `_`, and `-`.
The ID must match the game's SDK ID for per-game statistics to appear.
Games with missing, invalid, or duplicate IDs are reported and not added.
The sample metadata is generated and copied to the deployment folder by CMake;
rebuild existing deployments to update older metadata. Use **Refresh games**
to rescan after adding or fixing a game.

Build and test Release, or create a development ZIP:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\setup.ps1 -Action Test -Configuration Release -QtPath "C:\Qt\6.8.3\msvc2022_64"
powershell -NoProfile -ExecutionPolicy Bypass -File .\setup.ps1 -Action Package -Configuration Release -QtPath "C:\Qt\6.8.3\msvc2022_64"
```

The ZIP in `out\` is **not distribution-ready**. Before sharing it, verify
Qt, SFML and third-party licenses, matching source availability, and Windows
runtime dependencies. See the architecture's licensing section.

## VS Code and CI

Open the repository root in VS Code and install the Microsoft C/C++ extension.
Install Qt using the [official Qt installer](https://www.qt.io/download-open-source).
Select Qt 6.5 or newer, the **MSVC 2022 64-bit** kit, and the Qt Quick and
Qt Quick Controls components. The selected kit must also provide Qt Test and
`windeployqt.exe` (both are included in the standard desktop kit).

Set the user environment variable to the kit root in PowerShell. For example,
after installing Qt 6.8.3:

```powershell
[Environment]::SetEnvironmentVariable('QT_ROOT_DIR', 'C:\Qt\6.8.3\msvc2022_64', 'User')
```

Change the path to match your installation, then restart VS Code so its tasks
inherit the variable. Verify the kit before building:

```powershell
Test-Path "$env:QT_ROOT_DIR\lib\cmake\Qt6\Qt6Config.cmake"
Test-Path "$env:QT_ROOT_DIR\bin\windeployqt.exe"
```

Both commands should return `True`. `setup.ps1` passes this kit's `Qt6_DIR`
directly to CMake, preventing an old build cache from selecting a different
Qt installation. It also checks for Git, CMake, Visual Studio 2022 C++ tools,
and CTest.

Use **Terminal: Run Task** to run **Arcade: Build Debug**,
**Arcade: Build Release**, **Arcade: Test Debug**, or
**Arcade: Package Release**. Tasks run from the repository root and their
output is shown in a dedicated terminal. The Run and Debug panel provides
**Arcade: Debug Launcher** and **Arcade: Run Release**; each runs its matching
build task first, then starts `Deployment\Debug\ArcadeLauncher.exe` or
`Deployment\Release\ArcadeLauncher.exe` with that deployment directory as its
working directory. If a launch fails before the debugger starts, resolve any
failure reported by the pre-launch build task first. GitHub Actions builds
and tests both configurations on Windows.

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
architecture for profile format and limitations.

## Web arcade and Android

The separate web launcher offers a Phaser Snake game and a raw WebGL cube.
Snake uses arrow keys or on-screen direction buttons; drag the cube with a
mouse or touch and use its pause button to stop rotation. Each game has a
return link. The launcher shows local playtime for both samples and the best
Snake score. Playtime counts active, visible gameplay and is checkpointed
periodically; closing or suspending the app can lose the latest interval.

Install Node.js 22.12 or newer within the 22.x line, Node.js 24.x, or
Node.js 26+ and run from the repository root:

```powershell
cd Source\Web
npm.cmd ci
npm.cmd run dev
```

Open the address printed by Vite. Run `npm.cmd run build` for a production
build, `npm.cmd test` for unit tests, and `npm.cmd run test:e2e` for browser
tests (install Playwright Chromium with `npx.cmd playwright install chromium`,
or set `ARCADE_CHROMIUM_PATH` to an existing Chromium-based browser).
Preview the production build with `npm.cmd run preview`.

### Add a web game

Each browser game has a folder under `Source\Web\src\games`, with its
component, game logic, tests, and any game-specific assets. These folders
share the web launcher's Vite project; they are not independent builds.

1. Add a folder and game component, using `games\snake` or `games\cube` as
   an example. The game must run in a browser; native SFML executables cannot
   run directly in one.
2. Add a unique, stable lowercase ID, title, description, and hash route to
   `src\catalog.ts`. In `src\App.tsx`, add a lazy import, map the route to
   the game ID, and render the component in `GameView`.
3. Use the `active` and `setPlaying` props to pause gameplay when hidden and
   stop playtime tracking when gameplay pauses or ends. Follow Snake's
   `updateStats` usage to record a high score if needed, and add its display
   to the game card. Keep IDs stable so existing statistics remain associated
   with the game.
4. Add unit and browser tests and run the web build and tests above. Check
   touch controls, navigation, statistics, and offline behavior. If assets use
   extensions not included by the Workbox `globPatterns` in `vite.config.ts`,
   update that list as needed for offline use.

### Publish the web arcade

The GitHub Pages workflow in `.github\workflows\pages.yml` builds and tests
the web app on pushes to `main`, and can also be started manually from the
Actions tab. In the repository's **Settings > Pages**, choose **GitHub
Actions** as the source. Push the workflow and web source to `main` and check
the workflow's deployment result. The expected project URL is
`https://leito25.github.io/arcklauncher/`; the workflow's deployment output
provides the actual URL. The current relative Vite base supports the project
subpath and the Android build.

GitHub Pages serves static files and makes the site public. GitHub Free
requires a public repository for Pages. It does not provide a backend or
cross-device profile synchronization.

The PWA can cache the launcher and both games for offline use after its first
successful online visit. Installation and service workers require HTTPS or
localhost and a supporting browser. Statistics are stored in IndexedDB
separately for each browser or installed Android app; they are not synced with
the native launcher or other devices and can be removed with site data.
Storage errors are reported in the app.

For Android, install Android Studio and accept the Android SDK licenses
yourself, then install the Android SDK and required platforms through its SDK
Manager, including Android SDK Platform 35. Configure JDK 21 for Gradle
(Android Studio's bundled JDK can be used if it is version 21), and set up an
emulator or device. From `Source\Web` run:

```powershell
npm.cmd run android:sync
npm.cmd run android:open
```

Build and run the project in Android Studio. Alternatively, with the SDK
installed in its default Windows location, use a JDK 21 installation for a
command-line debug build (adjust the JDK path for your machine):

```powershell
$env:JAVA_HOME = 'C:\Program Files\Android\openjdk\jdk-21.0.8'
$env:ANDROID_HOME = "$env:LOCALAPPDATA\Android\Sdk"
cd android
.\gradlew.bat assembleDebug
& "$env:ANDROID_HOME\platform-tools\adb.exe" devices -l
& "$env:ANDROID_HOME\platform-tools\adb.exe" install -r app\build\outputs\apk\debug\app-debug.apk
& "$env:ANDROID_HOME\platform-tools\adb.exe" shell am start -n com.arcadeportfolio.app/.MainActivity
```

The install and start commands require one connected, booted emulator or USB
device. Hardware-accelerated emulators require firmware virtualization and a
working hypervisor; consult Android Studio's emulator setup if an AVD stays
offline. The debug APK has built on Windows and launched on an Android phone;
gameplay and WebGL on real devices remain unverified. A wrapper is not a
guarantee of compatibility. iOS, accounts and cross-device synchronization
are future work.

## Planning documents

- [Architecture and initial roadmap](arcade_portfolio_architecture.md)
- [Requirements and open decisions](docs/requirements.md)
- [Planning workflow](docs/workflow.md)
- [Implementation todos](docs/todos.md)

The architecture document is a starting proposal, not a finalized design.
In particular, storage, concurrency, platform support, and packaging still
need explicit decisions.

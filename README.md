# Arcade Portfolio

A planned cross-platform arcade collection with a launcher, standalone games,
and shared player statistics. The Qt launcher discovers games from metadata
and starts the SFML circle sample in a separate process. Player statistics and
the SDK are not yet implemented. Windows is the initial development target.

## Build on Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload,
CMake 3.22 or newer, Git, and Qt 6.5 or newer with the MSVC 2022 64-bit kit,
Qt Quick, and Qt Quick Controls. Use a Qt kit built for the same compiler and
architecture as the application. CMake downloads SFML 3.0.2 on first
configuration; an internet connection is required.

In PowerShell, replace the example Qt path with your installation:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\msvc2022_64"
cmake --build build --config Debug
```

Deploy the Qt runtime and QML plugins alongside the launcher, then run it:

```powershell
& "C:\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe" --debug --qmldir .\Source\Launcher .\Deployment\ArcadeLauncher.exe
.\Deployment\ArcadeLauncher.exe
```

Select **Play** to open the circle sample; closing it returns to the launcher.
The game also runs independently from
`.\Deployment\games\TemplateSFML\TemplateSFML.exe`.
The launcher scans `Deployment\games` relative to its own executable, not the
current working directory. Each game folder needs a `meta.json` with a
nonempty `title` and an `executable` filename in that same folder. The sample
metadata is generated and copied to the deployment folder by CMake. Use
**Refresh games** to rescan after adding or fixing a game.

The deployment folder is development output, not a redistributable package.
The build copies each executable there; `windeployqt` must be run separately
after a build to copy Qt's runtime libraries and QML plugins. For Release,
build with `--config Release` and use `windeployqt --release` instead.

To build only one component, configure with
`-DARCADE_BUILD_LAUNCHER=OFF` or `-DARCADE_BUILD_SFML_TEMPLATE=OFF`.
When the launcher is disabled, Qt is not required.

Run the launcher tests after building:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

## Planning documents

- [Architecture and initial roadmap](arcade_portfolio_architecture.md)
- [Requirements and open decisions](docs/requirements.md)
- [Planning workflow](docs/workflow.md)
- [Implementation todos](docs/todos.md)

The architecture document is a starting proposal, not a finalized design.
In particular, storage, concurrency, platform support, and packaging still
need explicit decisions.

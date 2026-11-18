# Arcade Portfolio: Architecture, Specifications & AI Prompt Guide

> **Status:** This document combines implemented features with proposed future
> architecture. Windows is the first development target. The
> current CMake workspace discovers and launches standalone SFML 2 and 3
> circle samples. The desktop SDK stores local statistics; the SFML 3 sample
> reports playtime on a normal exit. The launcher displays combined local
> playtime; web support remains future work. The setup script creates
> configuration-specific development deployments and a development-only ZIP;
> release compliance is unfinished. See
> [implementation todos](docs/todos.md).

## 1. Project Overview
A professional-grade, multi-platform arcade collection acting as a C++ portfolio showcase. 
*   **Launcher:** A modern Qt6 (QML) Launcher that dynamically discovers and runs standalone game executables.
*   **Games:** A planned collection of standalone desktop games and browser games.
*   **SDK:** A C++ library with a C ABI for local desktop statistics; a separate
    browser SDK and optional synchronization service are future work.
*   **Web launcher (planned):** A responsive website for desktop and mobile
    browsers, with an optional mobile wrapper later.

---

## 2. Workspace & Deployment Structure (Monorepo)
Your repository separates source code from the final compiled product. CMake copies compiled binaries into the `Deployment` folder.

```text
ArcadeWorkspace/               
├── CMakeLists.txt             <-- Master build script
├── Source/
│   ├── ArcadeSDK/             <-- Shared C-ABI library (.dll / .so)
│   ├── Launcher/              <-- Qt6 UI Application
│   └── Games/                 
│       ├── Template SFML3/    <-- SFML 3 template
│       ├── TemplateSFML2/     <-- SFML 2 template, separate CMake build
│       └── Tetris/            <-- planned game
└── Deployment/Debug/          <-- setup.ps1 also supports Release
    ├── ArcadeLauncher.exe
    └── games/
        └── Tetris/            <-- planned deployment example
            ├── Tetris.exe     
            ├── meta.json      <-- Launcher metadata (ID, title, executable)
            └── assets/        
```

---

## 3. Core Components

### A. The Arcade SDK (Data Layer)
*   **Role:** Local per-user playtime, high scores and achievements.
*   **Implementation:** A C++ shared library using `nlohmann/json` and a C ABI.
    Calls return explicit status codes; `ArcadeLastError` describes the latest
    error on the calling thread. IDs use lowercase ASCII letters, digits,
    `.`, `_` and `-`. One game session may be active per process.
*   **Location:** Windows uses `%LOCALAPPDATA%/ArcadePortfolio/profile.json`
    (the Windows LocalAppData known folder). Other desktop platforms use
    `$XDG_DATA_HOME/ArcadePortfolio`, or `$HOME/.local/share/ArcadePortfolio`
    if XDG is unset. `ArcadeInit` can instead receive an absolute UTF-8
    directory path. Windows is the initial tested target.
*   **Concurrency:** Cooperating SDK processes lock the persistent
    `profile.lock` using OS file locks, read and validate the latest profile,
    write a temporary file in the same directory, then replace the profile.
    Locks are released by the OS if a process crashes. This assumes a local
    filesystem with working locking and atomic replacement, not a network
    filesystem. Do not delete the lock file while applications are running.
    On Windows, replacement retries briefly if another program temporarily
    denies delete sharing; persistent failures are reported without overwriting
    the existing profile.
*   **Recovery:** Missing profiles are created. Corrupt or unsupported profiles
    are rejected without overwriting them; back up and repair them manually.
    A crash may leave an unused temporary file. The scheme protects against
    partial replacement but does not guarantee durability after power loss.
*   **Limits:** Playtime and high scores are nonnegative integers no greater
    than 9,007,199,254,740,991 (the JavaScript safe-integer limit). Scores keep
    the highest value; achievements are idempotent. These statistics are local
    and not tamper-proof. Forced termination can lose the current session's
    playtime.

The version 1 profile is the first shared **conceptual** format for desktop and
future web work. Additive fields can be preserved; incompatible changes require
a versioned migration, which is not implemented yet.

```json
{
  "schemaVersion": 1,
  "games": {
    "template-sfml3": {
      "playtimeSeconds": 12,
      "highScore": null,
      "achievements": ["first-win"]
    }
  }
}
```

### B. The Qt6 Launcher (Frontend)
*   **Role:** The entry point, displaying available games in a responsive grid
    with combined and per-game local playtime.
*   **Implementation:** C++ backend for OS logic, QML for GPU-accelerated UI animations.
*   **Mechanics:** 
    *   Uses Qt directory and JSON APIs to scan `games/` relative to the launcher for `meta.json` files.
    *   Uses `QProcess` to launch games as separate OS-level processes.
    *   Watches the local `profile.json`, validates version 1 playtime fields,
        and refreshes global and per-game playtime when the file changes.
        Cards show whole minutes or "Not played yet" when no entry exists.
    *   Requires a unique metadata ID using lowercase ASCII letters, digits,
        `.`, `_`, and `-`; the ID must match the SDK profile key. Missing,
        invalid, and duplicate IDs are reported and excluded from the grid.

### C. The Games (Executables)
*   **Role:** Standalone experiences.
*   **Implementation:** Standalone SFML 2 or SFML 3 games. Each executable uses
    exactly one version.
*   **Mechanics:** Completely independent of the launcher. The SFML 3 sample
    reports elapsed whole seconds on normal exit; SFML 2 integration and real
    game scores and achievements are future work. Game metadata includes a
    stable ID that matches the SDK profile key for per-game statistics.

### D. Web and mobile (planned)

*   Build a separate responsive launcher, initially suggested as React,
    TypeScript and Vite. It reads a web catalog and navigates to a selected
    game's URL; games provide a way back to the launcher. Web games may use
    Phaser, WebGL or another browser-compatible engine. Native SFML
    executables cannot run directly in a browser.
*   Keep stable game IDs and the same statistical meanings across platforms.
    A future browser SDK would store data locally (for example, IndexedDB);
    it cannot use the desktop DLL or its filesystem locks. Desktop metadata
    currently describes executable filenames; a separate web catalog will
    describe URLs until a shared versioned catalog is designed.
*   Start with responsive layout, touch input and a small Phaser sample.
    Consider a PWA for installation and, later, a Capacitor wrapper for Android
    and iOS. Test WebGL, performance and input in real mobile browsers and
    webviews; a wrapper alone does not ensure compatibility.
*   Cross-device statistics require a future backend, account and explicit
    conflict-resolution design. No web frontend, browser SDK, sync, accounts
    or mobile app is implemented yet.

---

## 4. GitHub Project Roadmap (Implementation Phases)

### Phase 1: Foundation & The Data Layer
*   [x] **Task 1.1:** Initialize Git repository and setup the root `CMakeLists.txt` monorepo structure.
*   [x] **Task 1.2:** Implement the initial C++ SDK, profile storage and C ABI.
*   [x] **Task 1.3:** Test SDK persistence, errors and concurrent processes.

### Phase 2: The Launcher Framework
*   [x] **Task 2.1:** Setup Qt6/QML project in CMake.
*   [x] **Task 2.2:** Write the C++ `GameScanner` class to read `meta.json` files from the `games/` folder.
*   [x] **Task 2.3:** Build the QML UI to bind the `GameScanner` data to a visual grid or carousel.
*   [x] **Task 2.4:** Implement `QProcess` in the C++ backend to launch executables.
*   [x] **Task 2.5:** Implement `QFileSystemWatcher` to read `profile.json` and display global playtime in the UI.

### Phase 3: Proving the End-to-End Loop
*   [x] **Task 3.1:** Create the `Template SFML3` project using SFML 3.0 via `FetchContent` in CMake.
*   [x] **Task 3.2:** Link the SFML 3 template to the `ArcadeSDK` to report playtime on normal exit.
*   [ ] **Task 3.3:** Build **Tetris** using the template.
*   [ ] **Task 3.4:** Integration Test: Launcher -> Plays Tetris -> Closes Tetris -> Launcher UI updates.

### Phase 4: Game Production (The 16 Classics)
*   [ ] **Task 4.1:** Grid & Logic (Snake, Minesweeper, 15 Puzzle, Match-3, NetWalk)
*   [ ] **Task 4.2:** Physics & Vectors (Asteroids, Arkanoid, Volleyball, Tron)
*   [ ] **Task 4.3:** Pathing & State (Chess, Mahjong)
*   [ ] **Task 4.4:** Camera & Math (DoodleJump, Outrun, Top-Down Racing, Xonix)

### Phase 5: Polish & CI/CD
*   [x] **Task 5.1:** Set up GitHub Actions for automated CMake builds on push.
*   [ ] **Task 5.2:** UI Polish (blur backgrounds, controller navigation, sound effects).
*   [ ] **Task 5.3:** Packaging scripts (Windows `.zip`, macOS `.app`, Linux `AppImage`).

### Phase 6: Web and mobile
*   [ ] Define a web catalog and implement a responsive launcher.
*   [ ] Create a Phaser sample with touch controls and a return-to-launcher path.
*   [ ] Implement local browser statistics using the shared concepts.
*   [ ] Evaluate PWA installation and a mobile wrapper on real devices.
*   [ ] Design optional accounts and cross-device synchronization.

### Build, onboarding, and licensing decisions

* Windows is the initial target. `setup.ps1` checks Git, CMake, Visual Studio
  2022 C++ tools, and an installed Qt 6 MSVC 2022 kit; it does not install
  tools or change system settings. It builds and deploys both sample games,
  optionally tests them, and can create a Release development ZIP.
* Qt is installed separately using the official installer, not built from a
  Git submodule. CMake FetchContent retrieves pinned SFML 2.6.2 and 3.0.2
  sources; the versions use independent CMake build trees so their targets
  and libraries do not conflict. Each game uses only one version and the
  normal upstream `SFML/` headers. Game executables communicate with the
  launcher as processes, not through SFML objects.
* `Deployment/Debug` and `Deployment/Release` keep runtime configurations
  separate. `windeployqt` copies Qt runtime plugins; CTest checks launcher
  behavior and game deployment. VS Code tasks and launch configurations wrap
  the script. GitHub Actions builds and tests Debug and Release on Windows.
* Each game template can also be opened as its own VS Code folder. Game-only
  builds use separate Debug and Release trees, require no Qt, and stage their
  own executable, metadata and dependencies. **Game: Debug** and **Game: Run
  Release** are launch configurations that invoke build tasks first; the
  per-game tasks also test, make a development ZIP, or explicitly deploy to
  the launcher. Both standalone and
  monorepo SFML 3 builds use the same pinned dependencies and SDK. The SFML 3
  source folder is named `Template SFML3`; its existing target and deployment
  folder remain `TemplateSFML` for compatibility.
* Qt offers LGPL/GPL and commercial licensing. The free LGPL option can be
  suitable for proprietary applications if its obligations are met. Some
  modules are GPL-only and Qt includes third-party components with their own
  licenses. Before distributing a build, check the exact Qt modules, SFML and
  all third-party terms; include required notices and license texts, provide
  matching Qt source as required, permit users to replace or relink Qt, and
  verify runtime dependencies. Dynamic linking helps but is not sufficient by
  itself. The ZIP is not a compliant release package yet. See [Qt licensing](https://doc.qt.io/qt-6/licensing.html)
  and [Qt's LGPL obligations](https://www.qt.io/licensing/open-source-lgpl-obligations).

---

## 5. AI Prompt Engineering Guide
*Use these precise prompts one by one in a fresh LLM context to generate the code for each phase without confusing the AI with too much scope.*

### Prompt 1: The Build System (For Task 1.1)
> "I am building a C++ monorepo with three main components: A shared SDK (C++), a Qt6 Launcher, and standalone SFML 3.0 games. Write the root `CMakeLists.txt` and the necessary subdirectory CMake files. The build system must: 
> 1. Use `FetchContent` for `nlohmann/json` (used by the SDK) and `SFML 3.0` (used by the games). 
> 2. Ensure Qt6 is linked properly for the Launcher. 
> 3. Include a `POST_BUILD` command for the Games target that automatically copies the compiled game executable into a root `Deployment/games/<GameName>/` directory so the Launcher can find it. Do not write the C++ code, just the CMake architecture."

### Prompt 2: The Arcade SDK (historical; superseded by section 3.A)
> "Write the C++ source code for a dynamic library called `ArcadeSDK`. It must export functions using `extern "C"` so it maintains a stable ABI. The SDK needs to manage a `profile.json` file located in `Deployment/data/`. Use `nlohmann/json`. It must include a robust physical file-locking mechanism (creating a `.lock` file) to prevent data corruption if multiple processes try to read/write simultaneously. Expose functions for `InitSDK(const char* gameId)`, `AddPlaytime(int seconds)`, `UnlockAchievement(const char* id)`, and `ShutdownSDK()`."

### Prompt 3: Qt Launcher Backend (historical; profile location is superseded by section 3.A)
> "Write the C++ backend for a Qt6 QML application that acts as a game launcher. I need a class exposed to QML via `Q_PROPERTY` and `Q_INVOKABLE`. The class must:
> 1. Scan a `Deployment/games/` directory using C++17 `<filesystem>`, reading `meta.json` inside each subfolder to get the game's title and executable name, storing them in a `QAbstractListModel`.
> 2. Contain a method to launch a selected game using `QProcess`. The launcher must hide its window while the process is running and show it again when the process finishes.
> 3. Use `QFileSystemWatcher` to monitor `Deployment/data/profile.json` and emit a Qt signal to the QML frontend to refresh player stats whenever the file is modified."

### Prompt 4: Qt Launcher UI (For Task 2.3)
> "Write the Qt6 QML code for a modern, Steam-like game launcher UI. It should consume a C++ `QAbstractListModel` containing game titles and icons. Display the games in a horizontal, smooth-scrolling ListView or PathView (carousel). Include a 'Play' button that calls a C++ `launchGame(QString path)` method. Add UI elements at the top of the screen to display global 'Hours Played' and 'Achievements Unlocked' which update dynamically via Qt signals."

### Prompt 5: SFML Game Integration (historical; SDK API is superseded by section 3.A)
> "Write the C++ boilerplate for a standalone SFML 3.0 game (a basic window with a game loop). This game must link dynamically to my `ArcadeSDK`. Show me how to call `InitSDK(\"tetris\")` before the game loop starts. Inside the game loop, track the delta time. When the user closes the SFML window, call `AddPlaytime(totalSeconds)` and then `ShutdownSDK()` before returning 0. Ensure the code cleanly handles the C-ABI boundary."
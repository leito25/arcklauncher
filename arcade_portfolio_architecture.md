# Arcade Portfolio: Architecture, Specifications & AI Prompt Guide

> **Status:** This is the initial proposed architecture, not a description of
> completed functionality. Windows is the first development target. The
> current CMake workspace discovers and launches a standalone SFML circle
> sample; SDK, shared statistics, and packaging remain to be designed and
> implemented. Executables are copied to `Deployment`, but deploying Qt
> dependencies requires the separate `windeployqt` step in the README. See
> [implementation todos](docs/todos.md).

## 1. Project Overview
A professional-grade, multi-platform arcade collection acting as a C++ portfolio showcase. 
*   **Launcher:** A modern Qt6 (QML) Launcher that dynamically discovers and runs standalone game executables.
*   **Games:** 16 standalone executables (built primarily in C++/SFML, but compatible with Unity/Unreal).
*   **SDK:** A custom C++ Arcade SDK that handles inter-process communication via a shared JSON data layer, tracking playtime, high scores, and achievements using strict file-locking.

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
│       ├── TemplateSFML/      <-- Base boilerplate for new games
│       └── Tetris/            
└── Deployment/                <-- Generated automatically by CMake
    ├── ArcadeLauncher.exe     
    ├── data/
    │   ├── profile.json       <-- Shared analytics data
    │   └── profile.lock       <-- SDK file-locking mechanism
    └── games/
        └── Tetris/
            ├── Tetris.exe     
            ├── meta.json      <-- Launcher metadata (Title, Icon path)
            └── assets/        
```

---

## 3. Core Components

### A. The Arcade SDK (Data Layer)
*   **Role:** Safely reads/writes to `profile.json`.
*   **Implementation:** C++ using `nlohmann/json`. Exports an `extern "C"` ABI so any language (C++, C#, Rust) can link to it safely.
*   **Concurrency:** Implements a physical `.lock` file to prevent the Launcher and the Games from corrupting the JSON if they read/write simultaneously.

### B. The Qt6 Launcher (Frontend)
*   **Role:** The entry point. A visual carousel of available games.
*   **Implementation:** C++ backend for OS logic, QML for GPU-accelerated UI animations.
*   **Mechanics:** 
    *   Uses C++17 `<filesystem>` to scan `Deployment/games/` for `meta.json` files.
    *   Uses `QProcess` to launch games as separate OS-level processes.
    *   Uses `QFileSystemWatcher` to monitor `profile.json` and instantly update UI stats when a game closes.

### C. The Games (Executables)
*   **Role:** Standalone experiences.
*   **Implementation:** SFML 3.0 (Graphics/Windowing/Audio) linked to the Arcade SDK.
*   **Mechanics:** Completely independent of the launcher. They report stats to the SDK upon Game Over or Exit.

---

## 4. GitHub Project Roadmap (Implementation Phases)

### Phase 1: Foundation & The Data Layer
*   [x] **Task 1.1:** Initialize Git repository and setup the root `CMakeLists.txt` monorepo structure.
*   [ ] **Task 1.2:** Implement `ArcadeSDK` (C++). Write the JSON parser, `.lock` file mutex, and `extern "C"` wrappers.
*   [ ] **Task 1.3:** Write a dummy C++ Console App to test linking to the SDK and writing to `profile.json`.

### Phase 2: The Launcher Framework
*   [x] **Task 2.1:** Setup Qt6/QML project in CMake.
*   [x] **Task 2.2:** Write the C++ `GameScanner` class to read `meta.json` files from the `games/` folder.
*   [ ] **Task 2.3:** Build the QML UI to bind the `GameScanner` data to a visual grid or carousel.
*   [x] **Task 2.4:** Implement `QProcess` in the C++ backend to launch executables.
*   [ ] **Task 2.5:** Implement `QFileSystemWatcher` to read `profile.json` and display global playtime in the UI.

### Phase 3: Proving the End-to-End Loop
*   [x] **Task 3.1:** Create `TemplateSFML` project using SFML 3.0 via `FetchContent` in CMake.
*   [ ] **Task 3.2:** Link `TemplateSFML` to the `ArcadeSDK` to report playtime on exit.
*   [ ] **Task 3.3:** Build **Tetris** using the template.
*   [ ] **Task 3.4:** Integration Test: Launcher -> Plays Tetris -> Closes Tetris -> Launcher UI updates.

### Phase 4: Game Production (The 16 Classics)
*   [ ] **Task 4.1:** Grid & Logic (Snake, Minesweeper, 15 Puzzle, Match-3, NetWalk)
*   [ ] **Task 4.2:** Physics & Vectors (Asteroids, Arkanoid, Volleyball, Tron)
*   [ ] **Task 4.3:** Pathing & State (Chess, Mahjong)
*   [ ] **Task 4.4:** Camera & Math (DoodleJump, Outrun, Top-Down Racing, Xonix)

### Phase 5: Polish & CI/CD
*   [ ] **Task 5.1:** Set up GitHub Actions for automated CMake builds on push.
*   [ ] **Task 5.2:** UI Polish (blur backgrounds, controller navigation, sound effects).
*   [ ] **Task 5.3:** Packaging scripts (Windows `.zip`, macOS `.app`, Linux `AppImage`).

---

## 5. AI Prompt Engineering Guide
*Use these precise prompts one by one in a fresh LLM context to generate the code for each phase without confusing the AI with too much scope.*

### Prompt 1: The Build System (For Task 1.1)
> "I am building a C++ monorepo with three main components: A shared SDK (C++), a Qt6 Launcher, and standalone SFML 3.0 games. Write the root `CMakeLists.txt` and the necessary subdirectory CMake files. The build system must: 
> 1. Use `FetchContent` for `nlohmann/json` (used by the SDK) and `SFML 3.0` (used by the games). 
> 2. Ensure Qt6 is linked properly for the Launcher. 
> 3. Include a `POST_BUILD` command for the Games target that automatically copies the compiled game executable into a root `Deployment/games/<GameName>/` directory so the Launcher can find it. Do not write the C++ code, just the CMake architecture."

### Prompt 2: The Arcade SDK (For Task 1.2)
> "Write the C++ source code for a dynamic library called `ArcadeSDK`. It must export functions using `extern "C"` so it maintains a stable ABI. The SDK needs to manage a `profile.json` file located in `Deployment/data/`. Use `nlohmann/json`. It must include a robust physical file-locking mechanism (creating a `.lock` file) to prevent data corruption if multiple processes try to read/write simultaneously. Expose functions for `InitSDK(const char* gameId)`, `AddPlaytime(int seconds)`, `UnlockAchievement(const char* id)`, and `ShutdownSDK()`."

### Prompt 3: Qt Launcher Backend (For Tasks 2.2 & 2.4 & 2.5)
> "Write the C++ backend for a Qt6 QML application that acts as a game launcher. I need a class exposed to QML via `Q_PROPERTY` and `Q_INVOKABLE`. The class must:
> 1. Scan a `Deployment/games/` directory using C++17 `<filesystem>`, reading `meta.json` inside each subfolder to get the game's title and executable name, storing them in a `QAbstractListModel`.
> 2. Contain a method to launch a selected game using `QProcess`. The launcher must hide its window while the process is running and show it again when the process finishes.
> 3. Use `QFileSystemWatcher` to monitor `Deployment/data/profile.json` and emit a Qt signal to the QML frontend to refresh player stats whenever the file is modified."

### Prompt 4: Qt Launcher UI (For Task 2.3)
> "Write the Qt6 QML code for a modern, Steam-like game launcher UI. It should consume a C++ `QAbstractListModel` containing game titles and icons. Display the games in a horizontal, smooth-scrolling ListView or PathView (carousel). Include a 'Play' button that calls a C++ `launchGame(QString path)` method. Add UI elements at the top of the screen to display global 'Hours Played' and 'Achievements Unlocked' which update dynamically via Qt signals."

### Prompt 5: SFML Game Integration (For Task 3.1 & 3.2)
> "Write the C++ boilerplate for a standalone SFML 3.0 game (a basic window with a game loop). This game must link dynamically to my `ArcadeSDK`. Show me how to call `InitSDK(\"tetris\")` before the game loop starts. Inside the game loop, track the delta time. When the user closes the SFML window, call `AddPlaytime(totalSeconds)` and then `ShutdownSDK()` before returning 0. Ensure the code cleanly handles the C-ABI boundary."
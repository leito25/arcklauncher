# Implementation todos

## Foundation

- [x] Add a CMake workspace with Qt Quick and an SFML 3 template.
- [x] Discover and launch an SFML circle sample from the Qt launcher.
- [ ] Decide the first playable game's scope and acceptance criteria ([#1](https://github.com/leito25/arcklauncher/issues/1)).
- [ ] Document supported Windows versions and development prerequisites ([#2](https://github.com/leito25/arcklauncher/issues/2)).

## Launcher and games

- [ ] Finalize the metadata schema, including icons and versioning ([#3](https://github.com/leito25/arcklauncher/issues/3)).
- [x] Discover basic title/executable metadata and handle launch failures.
- [ ] Create the first playable game and test the end-to-end loop.

## Player statistics and SDK

- [x] Implement the initial local profile schema, locking, recovery behavior and C ABI.
- [x] Record SFML 3 sample playtime on normal exit.
- [ ] Integrate SFML 2 and real game scores and achievements.
- [ ] Display profile statistics in the launcher.
- [ ] Design profile migrations and cross-platform validation ([#4](https://github.com/leito25/arcklauncher/issues/4), [#5](https://github.com/leito25/arcklauncher/issues/5)).

## Web and mobile (planned)

- [ ] Design a web game catalog and responsive launcher.
- [ ] Build a browser game sample with touch controls and local statistics.
- [ ] Evaluate PWA and mobile wrappers; design optional cross-device sync.

## Distribution

- [x] Add Windows automated builds and tests for both SFML samples.
- [x] Add Windows setup, VS Code tasks, and a development ZIP workflow.
- [x] Add per-game Debug, Release, testing, packaging and launcher deployment workflows.
- [ ] Define packaging, dependency deployment, and licensing for each platform.

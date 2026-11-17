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

- [ ] Design profile storage, schema, locking, and recovery ([#4](https://github.com/leito25/arcklauncher/issues/4)).
- [ ] Define and implement the SDK integration contract ([#5](https://github.com/leito25/arcklauncher/issues/5)).
- [ ] Track and display playtime, high scores, and achievements.

## Distribution

- [ ] Add automated builds and tests.
- [ ] Define packaging, dependency deployment, and licensing for each platform.

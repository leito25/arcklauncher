# Requirements and open decisions

Status: draft. These requirements summarize the initial architecture proposal;
details and acceptance criteria should be refined before implementation.

## Vision

Create a cross-platform arcade collection that showcases independent games
through a shared launcher and player profile.

## Proposed capabilities

- Discover installed games using metadata and launch them as separate processes.
- Allow games to run independently of the launcher.
- Track player statistics, including playtime, high scores, and achievements.
- Present available games and player statistics in the launcher.
- Support a first end-to-end experience before expanding the game collection.

## Initial scope

The first implementation milestone should demonstrate one game being
discovered, launched, and reporting statistics that the launcher can display.
The full collection, controller support, visual polish, and distributable
packages can follow after the basic integration is proven.

## Decisions to make

- Which Windows versions are supported for development and release?
- What are the game metadata and player profile schemas and their versioning
  and migration rules?
- Where should writable user data live on each supported platform?
- How will simultaneous processes safely update data, including after a crash?
- How will SDK errors, missing data, corrupted data, and recovery be handled?
- What is the SDK's supported C ABI, ownership model, and versioning policy?
- What are the launcher behavior and user feedback when a game fails to start?
- How will dependencies, licenses, and third-party game assets be managed?
- What are the acceptance criteria for the first playable end-to-end slice?

See the [initial architecture](../arcade_portfolio_architecture.md) for the
proposed technologies and longer-term roadmap.

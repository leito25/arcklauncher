# Planning workflow

## Organizing work

Use the repository for versioned specifications and GitHub Issues for
questions, decisions, and actionable work. A GitHub Project board can group
issues into Backlog, Ready, In progress, and Done. Use milestones for
outcomes rather than dates until a schedule is agreed.

Suggested initial milestones:

1. **Planning:** clarify requirements and record architectural decisions.
2. **First playable slice:** one game, launcher discovery and launch, and
   player statistics working end to end.
3. **Collection and polish:** expand games and refine the experience.
4. **Distribution:** agree supported platforms and package releases.

## Making changes

1. Create an issue describing the problem or decision and its acceptance
   criteria.
2. Make a focused change to the relevant document. For larger changes, use a
   branch and pull request to make review and discussion easier.
3. Update related documents and link the change to the issue.
4. Close the issue when its acceptance criteria are met.

Automated builds and tests can be introduced when implementation begins.
Do not commit generated builds, packaged output, or personal player profiles.

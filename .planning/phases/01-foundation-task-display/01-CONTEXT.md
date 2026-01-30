# Phase 1: Foundation & Task Display - Context

**Gathered:** 2026-01-29
**Status:** Ready for planning

<domain>
## Phase Boundary

Display all Todoist tasks in a scrollable list on the reMarkable 2's e-ink screen. User can see task name, due date, project, and priority for each task. Checkboxes are visible but task completion is Phase 2. Auth token is stored in plaintext config file and loaded on startup.

</domain>

<decisions>
## Implementation Decisions

### Claude's Discretion

User chose to skip detailed discussion — Claude has flexibility on all visual and interaction decisions:

- **Task list layout:** Row-based vs card-based, spacing, visual density
- **Information hierarchy:** What's prominent per task, how metadata is arranged
- **Priority indicators:** How P1-P4 are visually distinguished (icons, markers, bold)
- **Touch targets:** Checkbox size, tap area sizing for stylus
- **Empty states:** What shows when no tasks exist
- **Error states:** How to display auth failures, network issues
- **E-ink optimization:** Refresh strategy, contrast choices

**Guiding principle:** Design for e-ink constraints (high contrast, minimize refreshes) and stylus input (reasonable touch targets). Keep it simple and functional.

</decisions>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches for e-ink task list UIs.

Research recommendations from STACK.md:
- Qt 5.15 with Widgets (pre-installed on device)
- High contrast black/white design
- Large touch targets (44x44px minimum)
- Static layout to help Qt optimize refreshes

</specifics>

<deferred>
## Deferred Ideas

None — discussion was skipped, no scope creep occurred.

</deferred>

---

*Phase: 01-foundation-task-display*
*Context gathered: 2026-01-29*

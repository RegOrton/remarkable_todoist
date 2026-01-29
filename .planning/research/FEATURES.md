# Feature Landscape

**Domain:** reMarkable 2 Todoist Client
**Researched:** 2026-01-29
**Confidence:** MEDIUM

## Table Stakes

Features users expect. Missing = product feels incomplete.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| View all tasks | Core purpose of task manager | Low | List view with name, due date, project, priority per spec |
| Check off tasks | Essential interaction for task completion | Low | Checkbox UI, API call to complete task |
| Manual sync/refresh | Users need to update task list on demand | Low | Pull from Todoist REST API, refresh display |
| Add new tasks | Can't be read-only, users need to create tasks | Medium | Handwriting recognition + form fields |
| Offline queue | E-ink devices used away from WiFi | Medium | Local storage for pending actions, sync when online |
| Due date display | Critical context for task prioritization | Low | Parse and display Todoist due date format |
| Project organization | Todoist's primary organizational unit | Low | Group/filter tasks by project |
| Priority indicators | Visual task importance (P1-P4) | Low | Color/icon system for priority levels |
| Battery efficiency | E-ink users expect multi-day battery life | Low | Static display, minimal refreshes, no animations |
| Fast startup | Users want quick access to task list | Medium | Local cache, background sync |

## Differentiators

Features that set product apart. Not expected, but valued.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Handwriting-first input | Leverages reMarkable's core strength | High | Natural for reMarkable users, aligns with device philosophy |
| Distraction-free focus | No notifications, pure task view | Low | Embrace e-ink constraints as feature |
| One-handed operation | Optimized for tablet reading posture | Medium | Large touch targets, minimal navigation depth |
| Toltec/Oxide integration | Native feel, launcher integration | Medium | Follows reMarkable app ecosystem patterns |
| Smart offline mode | Transparent online/offline transitions | Medium | Show sync status subtly, queue gracefully |
| Natural language parsing | "Submit report Friday" → structured task | Medium | Todoist Quick Add API endpoint supports this |
| Template-based input | Pre-filled common tasks (meetings, calls) | Low | Reduces handwriting, speeds task creation |
| Minimal refresh strategy | Only refresh changed screen regions | High | Reduces e-ink flashing, improves UX |
| Label support | Additional organization beyond projects | Low | Todoist labels for cross-project categorization |
| Gesture-based shortcuts | Swipe to complete, long-press for details | Medium | Reduces UI chrome, faster interactions |

## Anti-Features

Features to explicitly NOT build. Common mistakes in this domain.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| Rich task editing | Breaks focus, complex UI unsuited to e-ink | Keep editing minimal. Full editing via web/mobile app |
| Push notifications | Defeats distraction-free purpose, e-ink can't do well | Manual refresh only. This is a pull-only client |
| Color-coded everything | E-ink is grayscale, color reduces readability | Use icons, text labels, patterns instead |
| Animations/transitions | E-ink ghosting makes these look terrible | Instant state changes with full/partial refresh |
| Commenting/collaboration | Scope creep, requires complex UI | Read-only for comments. Add tasks only |
| Custom filters | Overwhelming, rarely used, complex UI | Use Todoist's built-in projects/labels only |
| Calendar view | Complex layout, poor e-ink fit, high dev cost | Simple list with due dates is sufficient |
| Recurring task management | Complex UI, edge cases, maintenance burden | Read/complete recurrences, create via web app |
| Attachment handling | File management is out of scope | Display attachment indicator, no upload/view |
| Multiple accounts | Adds complexity, minimal user value | Single Todoist account only |
| Full CRUD for projects | Organizational changes done elsewhere | Read-only projects, manage via Todoist apps |

## Feature Dependencies

```
Core Dependencies:
Authentication → All features (must login to Todoist first)
Task retrieval → Task list display
Task list display → Task completion, offline queue
Handwriting recognition → Task creation

Phase Structure:
Phase 1: Read-only (authentication + task list + manual sync)
Phase 2: Task completion (checkboxes + offline queue + sync)
Phase 3: Task creation (handwriting + form + Quick Add API)

Optional Enhancements:
Natural language parsing depends on Task creation
Templates depend on Task creation
Gesture shortcuts depend on Task list display
Partial refresh depends on Task list display
Labels depend on Task retrieval
```

## MVP Recommendation

For MVP, prioritize:

1. **Authentication** (OAuth or API token input) - Can't do anything without it
2. **Task list display** (name, due date, project, priority) - Core value proposition
3. **Manual sync** (pull latest tasks from API) - Keep data current
4. **Task completion** (checkboxes, mark done) - Minimum interaction needed
5. **Offline queue** (store actions when offline) - Handle reMarkable's mobile context

Defer to post-MVP:

- **Task creation**: High complexity (handwriting recognition), less critical than viewing/completing
- **Natural language parsing**: Nice enhancement to task creation, not essential
- **Labels**: Additional organization, can work with projects first
- **Gesture shortcuts**: UX refinement, not core functionality
- **Partial refresh optimization**: Performance polish, full refresh works initially
- **Templates**: Convenience feature for power users

## E-Ink Specific Considerations

reMarkable 2's e-ink display imposes unique constraints that shape feature viability:

| Constraint | Impact on Features | Design Response |
|------------|-------------------|----------------|
| Slow refresh (200-400ms) | No animations, minimal transitions | Instant state changes, static UI |
| Grayscale only | No color-coded priorities/projects | Icons, text labels, patterns |
| Low contrast | Hard to read small text | Large fonts, high contrast elements |
| Ghosting on partial refresh | UI artifacts from previous state | Strategic full/partial refresh balance |
| No backlight | Ambient light dependency | High contrast black/white design |
| Battery life expectations | Multi-day usage expected | Minimize refresh frequency, static display |

## Todoist API Coverage

Based on REST API v2 capabilities, feature support:

| Feature Category | API Support | Implementation Notes |
|-----------------|-------------|---------------------|
| Task CRUD | Full | Create, read, update, delete tasks |
| Task completion | Yes | POST to close, reopen endpoints |
| Projects | Read + create | Read-only recommended for this client |
| Sections | Full | Useful for project organization |
| Labels | Full | Personal + shared labels supported |
| Comments | Read + create | Read-only recommended to avoid complexity |
| Filters | Limited | Single filter only, no comma operators |
| Due dates | Full | datetime + timezone support |
| Priority | Full | 1-4 scale (maps to P1-P4) |
| Natural language | Yes | Quick Add endpoint for parsing |
| Collaboration | Read-only | View assignees, no assignment |

## Handwriting Recognition Scope

reMarkable has built-in OCR, but constraints apply:

| Aspect | Status | Implementation Strategy |
|--------|--------|-------------------------|
| On-device OCR | Available | Use reMarkable's native OCR API |
| Cloud OCR | Via Connect subscription | Fallback if user has subscription |
| Recognition quality | Variable | Validate input, allow correction |
| Multi-language | Supported | Follow user's reMarkable language setting |
| Supported input | Task name only | Keep scope minimal per spec |
| Format recognition | Limited | Expect plain text output only |

**Recommendation**: Use on-device OCR for task names only. Pre-validation with structured form for due date, project, priority reduces OCR complexity and errors.

## Offline-First Design Patterns

Based on offline task manager research:

| Pattern | Application | Complexity |
|---------|-------------|------------|
| Local-first data | Cache all tasks locally, sync in background | Medium |
| Optimistic UI | Show completion immediately, sync later | Low |
| Conflict resolution | Last-write-wins for task completion | Low |
| Sync status indicator | Subtle icon showing online/offline/syncing | Low |
| Queue persistence | Store pending actions across app restarts | Medium |
| Background sync | Auto-sync when connection detected | Medium |
| Manual refresh | User-initiated sync for immediate update | Low |

**Privacy benefit**: Local storage means less cloud exposure, aligns with reMarkable's focus philosophy.

## Competitive Landscape Context

reMarkable productivity apps follow patterns:

| Pattern | Description | Application to Todoist Client |
|---------|-------------|-------------------------------|
| Template-based | Pre-formatted pages (checklists, planners) | Use for task creation templates |
| Document-centric | Convert external content to PDF/ePUB | Not applicable (API-driven tasks) |
| Minimal UI | Limited navigation, focus on content | Embrace for task list design |
| Offline-capable | SSH access, local storage | Essential for mobile usage |
| Launcher integration | Oxide/Toltec distribution | Follow for native feel |
| No cloud dependency | Work without network | Support but don't require |

**Key insight**: reMarkable users value simplicity, offline capability, and distraction-free focus over feature richness.

## Sources

**reMarkable ecosystem and constraints:**
- [reMarkable: 5 Features for 2026 Productivity](https://brandenbodendorfer.com/5-remarkable-features-new-users-might-not-know/)
- [Boost your productivity: plan and achieve goals with reMarkable](https://remarkable.com/using-remarkable/workflows/reach-your-goals-with-planners)
- [GitHub - reHackable/awesome-reMarkable](https://github.com/reHackable/awesome-reMarkable)
- [How to Design for E-Ink Devices?](https://www.withintent.com/blog/e-ink-design/)
- [These are the challenges for developing apps for an Android e-Reader](https://goodereader.com/blog/electronic-readers/these-are-the-challenges-for-developing-apps-for-an-android-e-reader)

**Todoist features and API:**
- [Changelog entries from 2026](https://www.todoist.com/help/articles/changelog-entries-from-2026-HD3jJAtLd)
- [Todoist Software 2026: Features, Integrations, Pros & Cons](https://www.capterra.com/p/149339/Todoist-for-Business/)
- [REST API Reference | Todoist Developer](https://developer.todoist.com/rest/v2/)
- [Guides | Todoist Developer](https://developer.todoist.com/guides/)

**Task management patterns:**
- [7 reasons Google Tasks is the best minimalist task management app](https://www.xda-developers.com/reasons-google-tasks-is-the-best-minimalist-task-management-app/)
- [What is offline task manager? – Focuskeeper Glossary](https://focuskeeper.co/glossary/what-is-offline-task-manager)
- [Brisqi — Offline-first Personal Kanban App](https://brisqi.com/)

**Handwriting recognition:**
- [reMarkable Handwriting Search in 2026: Key2Success Planner](https://brandenbodendorfer.com/how-to-search-handwritten-notes-in-remarkable/)
- [Convert Handwriting to Text on Remarkable 2 | OCR Guide](https://www.handwritingocr.com/handwriting-to-text/how-to-convert-handwriting-to-text-on-remarkable-2)
- [GitHub - akeil/rescript: Handwriting recognition for reMarkable notes](https://github.com/akeil/rescript)

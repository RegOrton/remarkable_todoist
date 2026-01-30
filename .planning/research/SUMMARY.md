# Project Research Summary

**Project:** reMarkable 2 Todoist Client
**Domain:** E-ink tablet productivity application with cloud sync
**Researched:** 2026-01-29
**Confidence:** MEDIUM-HIGH

## Executive Summary

This project involves building a native task management application for the reMarkable 2 e-ink tablet that syncs with Todoist. Experts build this type of application by combining embedded device programming (C++ with Qt framework), offline-first sync architecture for mobile contexts, and e-ink display optimization techniques. The reMarkable ecosystem is well-documented with active community support through Toltec package management and Oxide launcher integration.

The recommended approach is to use Qt 5.15 (pre-installed on device), implement an offline-first architecture with command queue persistence, and prioritize manual sync over background services to preserve battery life. Start with read-only features (task list viewing) and layer on write operations (task completion, then creation) progressively. The application should embrace e-ink constraints as features rather than fighting them — static UI, high contrast, distraction-free focus.

Key risks include device soft-bricking from unsupported rm2fb versions, silent data loss from naive conflict resolution, and battery drain from aggressive sync strategies. Mitigate these by documenting supported OS versions with runtime checks, implementing field-level merge logic with operation tracking, and defaulting to manual sync with conservative timers. Handwriting recognition is technically complex and should be deferred to post-MVP; start with keyboard input for task creation.

## Key Findings

### Recommended Stack

The reMarkable 2 development ecosystem is mature and well-supported. C++ with Qt 5.15 is the clear choice for this application because Qt comes pre-installed on the device with excellent networking (QNetworkAccessManager) and JSON (QJsonDocument) support built-in. The alternative frameworks like rmKit require custom framebuffer handling and lack mature HTTP clients, making them unsuitable for API-heavy applications.

**Core technologies:**
- **C++17 with Qt 5.15**: Native performance, pre-installed framework with networking and JSON — no reason to use alternatives since Qt handles all requirements
- **Todoist REST API v2**: Simple REST endpoints for task CRUD — avoid Sync API v9 which is overkill for this read-heavy workload
- **JSON file storage**: Lightweight persistence for tasks and offline queue — SQLite is overkill for <1000 tasks, JSON is debuggable
- **rm2fb display package**: REQUIRED on reMarkable 2 for framebuffer access — without this the app cannot render to screen
- **Toltec/Oxide ecosystem**: Standard distribution via opkg packages — community-accepted deployment method with launcher integration
- **CMake + Docker toolchain**: Cross-compilation using official toltec-dev Docker images — standardized build process for ARMv7 target

**Critical constraint:** Handwriting recognition requires MyScript Cloud API with paid developer account and HMAC authentication. On-device OCR exists but quality is variable. Recommendation: defer handwriting to v2, start with Qt keyboard input.

### Expected Features

Research reveals that e-ink task managers succeed when they embrace device constraints rather than attempting to replicate feature-rich mobile apps. Users value simplicity, offline capability, and distraction-free focus over comprehensive feature sets.

**Must have (table stakes):**
- View task list (name, due date, project, priority) — core value proposition
- Check off tasks to mark complete — minimum interaction needed
- Manual sync/refresh button — keep data current
- Offline queue for actions — handle mobile context without WiFi
- Due date and priority display — critical task context
- Project organization — Todoist's primary grouping mechanism

**Should have (competitive):**
- Handwriting-first task input — leverages reMarkable's core strength, but HIGH complexity
- Add new tasks via form — fallback if handwriting deferred
- Distraction-free focus mode — embrace e-ink constraints as feature
- Smart offline mode — transparent online/offline transitions with subtle status indicator
- Label support — additional organization beyond projects

**Defer (v2+):**
- Rich task editing (notes, subtasks, attachments) — breaks focus, complex UI unsuited to e-ink
- Background auto-sync — battery drain risk, manual sync sufficient for MVP
- Natural language parsing — enhancement to task creation, not essential
- Recurring task management — complex edge cases, create via web app instead
- Calendar view — complex layout, poor e-ink fit
- Multi-account support — adds complexity with minimal user value

**Anti-features (explicitly avoid):**
- Push notifications — defeats distraction-free purpose, e-ink can't do well
- Animations/transitions — e-ink ghosting makes these look terrible
- Color-coded priorities — device is grayscale, use icons/patterns instead
- Background services keeping WiFi on — ruins battery life

### Architecture Approach

The architecture combines three domains: Qt application structure (Model-View-Controller), offline-first sync (command queue with conflict resolution), and e-ink display optimization (partial framebuffer updates). The design uses Qt's signals/slots for inter-component communication, worker threads for blocking operations (network, handwriting), and JSON files for durable persistence.

**Major components:**
1. **TaskManager** — In-memory cache of tasks/projects, filter/sort operations, central data coordinator
2. **JSONStore** — Atomic file I/O for tasks.json and offline queue persistence
3. **SyncEngine** — Runs on QThread, handles REST API calls with exponential backoff, processes command queue with UUID-based idempotency
4. **ViewController** — Coordinates views, routes user actions to business logic, exposes Q_INVOKABLE methods for QML
5. **TaskListView** — Display layer with partial e-ink updates for individual task changes
6. **rm2fb integration** — Framebuffer shim required for reMarkable 2 display (rM1-style calls translated to SWTCON API)

**Key patterns:**
- Single-threaded UI on main thread, background sync on worker thread
- Offline-first: all modifications queue locally first, sync asynchronously
- Partial framebuffer updates for checkboxes, full refresh every 5-10 partials or screen transitions
- Command queue with UUIDs for retry idempotency
- Atomic JSON writes via temp file + rename to prevent corruption

**Component build order:** Data models → JSONStore → TaskManager → SyncEngine → Controller → UI views. This enables incremental testing with desktop unit tests before on-device deployment.

### Critical Pitfalls

1. **Soft-bricking device with unsupported rm2fb version** — Installing rm2fb on unsupported Xochitl firmware makes screen non-functional. ALWAYS check rm2fb compatibility matrix, install via Toltec package manager only, document supported OS versions (2.6-3.3 as of 2025), add runtime version checks
2. **E-ink ghosting from insufficient full refreshes** — Using only partial updates causes visual residue that makes UI illegible. Perform full refresh every 5 partial updates, at screen transitions, and user-triggered sync. Make refresh frequency configurable
3. **Silent data loss from last-write-wins conflict resolution** — Naive timestamp-based sync overwrites user edits without warning. Use field-level updates not document overwrites, completion status always wins over field edits, track operations with monotonic IDs, show conflicts to user
4. **Hardcoded API tokens in source** — Embedded secrets are easily extracted from binaries, compromising user accounts. Store tokens in ~/.config with chmod 600, prompt on first run, never commit credentials, use .gitignore
5. **No exponential backoff for rate limits** — Hitting Todoist's 450 req/15min limit causes retry loops that drain battery. Implement exponential backoff with jitter (1s → 60s max), respect Retry-After header, max 5 retries, only retry 429/5xx not 4xx
6. **Touch targets too small for stylus** — 24x24px checkboxes cause missed taps and frustration on e-ink with no immediate feedback. Minimum 48x48dp touch targets (9mm physical), make entire task row tappable, test with actual stylus on device
7. **Handwriting recognition assuming perfect OCR** — Treating OCR as infallible creates tasks with wrong names user can't fix. Show editable OCR output BEFORE submission, provide on-screen keyboard for corrections, test with diverse handwriting samples
8. **Background sync draining battery** — Auto-sync keeping WiFi active drops battery from 2 weeks to 2 days. Default to manual sync, turn WiFi off after sync completes, don't wake from sleep, use systemd timer not daemon if auto-sync enabled

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Foundation & Environment Setup
**Rationale:** Establish development environment and core data models before any device-specific work. This phase can be built and tested entirely on desktop, enabling rapid iteration without on-device deployment overhead.
**Delivers:** Cross-compilation toolchain, project structure, data models (Task, Project), JSONStore with atomic writes, TaskManager with in-memory cache, desktop unit tests
**Addresses:** Secure token storage pattern (pitfall #4), establishes patterns to avoid memory leaks (pitfall #11)
**Avoids:** Starting with device-specific code before data layer is solid
**Research flag:** SKIP — standard Qt/C++ patterns, well-documented

### Phase 2: Display Layer & Basic UI
**Rationale:** Build read-only UI with proper e-ink refresh management before adding interactivity. This validates rm2fb integration, touch target sizing, and grayscale design decisions early.
**Delivers:** rm2fb framebuffer integration, TaskListView with partial/full refresh strategy, proper touch target sizing (48x48dp), high-contrast grayscale UI, on-device deployment
**Addresses:** Table stakes features (view task list, due dates, priorities, projects)
**Avoids:** E-ink ghosting (pitfall #2), touch targets too small (pitfall #6), color-dependent UI (pitfall #12)
**Research flag:** MEDIUM — rm2fb partial update tuning requires empirical testing on device

### Phase 3: Todoist API Integration & Sync
**Rationale:** Add network layer and sync logic after UI is proven functional. Offline-first design means UI already works with cached data, network is purely additive.
**Delivers:** SyncEngine on worker thread, QNetworkAccessManager integration, REST API v2 client with Bearer auth, exponential backoff with rate limit handling, manual sync trigger, conflict resolution with field-level merging
**Addresses:** Manual sync (table stakes), smart offline mode (differentiator)
**Avoids:** Synchronous network calls blocking UI (anti-pattern #1), no rate limit handling (pitfall #5), conflict resolution data loss (pitfall #3)
**Research flag:** SKIP — well-documented REST patterns and offline-first architecture

### Phase 4: Task Completion & Offline Queue
**Rationale:** Write operations build on proven sync foundation. Offline queue is critical for mobile context without WiFi.
**Delivers:** Task completion with optimistic UI updates, offline command queue with UUID-based idempotency, queue persistence across app restarts, queue processing with retry logic
**Addresses:** Check off tasks (table stakes), offline queue (table stakes)
**Avoids:** No conflict handling (pitfall #3), failed request accumulation
**Research flag:** SKIP — standard queue patterns

### Phase 5: Task Creation (Keyboard Input)
**Rationale:** Start with simple keyboard input rather than handwriting. This delivers task creation capability without blocking on complex OCR integration.
**Delivers:** AddTaskView with Qt virtual keyboard, form fields for task name/due date/project/priority, validation and error handling, Quick Add API endpoint for natural language parsing (optional)
**Addresses:** Add new tasks (table stakes), natural language parsing (should have, if Quick Add works)
**Avoids:** Handwriting recognition complexity (pitfall #7)
**Research flag:** SKIP — standard form input

### Phase 6: Packaging & Distribution
**Rationale:** Polish and prepare for user deployment after core functionality is proven.
**Delivers:** Oxide app registration, Toltec package (.ipk), OS version compatibility checks, icon and metadata, installation documentation, error handling and user feedback
**Addresses:** Toltec/Oxide integration (differentiator)
**Avoids:** Soft-bricking unsupported devices (pitfall #1)
**Research flag:** LOW — Toltec packaging is documented but may need troubleshooting

### Phase 7: Handwriting Recognition (Post-MVP)
**Rationale:** Defer to v2 after validating core app with users. High complexity, uncertain OCR quality, requires paid MyScript account or custom recognizer.
**Delivers:** Handwriting capture via Oxide API, OCR integration (MyScript or on-device), editable OCR output before submission, confidence scoring and review workflow
**Addresses:** Handwriting-first input (differentiator, HIGH value but HIGH risk)
**Avoids:** Assuming perfect OCR (pitfall #7)
**Research flag:** HIGH — MyScript API integration and accuracy are unknown, may need fallback strategy

### Phase 8: Polish & Optional Enhancements
**Rationale:** User feedback drives priority after MVP launch.
**Delivers:** Label support, gesture shortcuts (swipe to complete), partial refresh tuning, battery-saving settings, template-based input, background sync (optional)
**Addresses:** Labels (should have), gestures (should have)
**Avoids:** Background sync battery drain (pitfall #8) — make opt-in with conservative defaults
**Research flag:** SKIP — refinements based on proven patterns

### Phase Ordering Rationale

- **Foundation first (Phase 1):** Data layer and business logic can be unit tested on desktop without device access, enabling rapid iteration
- **Display before sync (Phase 2 → 3):** Validates e-ink constraints and UI patterns early with static data before adding network complexity
- **Read before write (Phases 2-3 → 4):** Viewing tasks is simpler than modifying them, offline queue depends on proven sync architecture
- **Simple creation before handwriting (Phase 5 → 7):** Keyboard input delivers task creation capability while deferring OCR complexity to post-MVP
- **Packaging after core features (Phase 6):** Distribution logistics handled once functionality is validated
- **Handwriting and polish as v2 (Phases 7-8):** High-risk/complexity features deferred until core app proves market fit

This ordering follows dependency chains from architecture research: Models → Storage → Sync → UI → Input. It also frontloads critical pitfalls (device compatibility, e-ink refresh, secure storage) before they become expensive to fix.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 7 (Handwriting):** MyScript API authentication, accuracy benchmarks, stroke data format, pricing — sparse documentation, may need proof-of-concept
- **Phase 2 (Display):** rm2fb waveform mode tuning, partial update performance at different task list sizes — requires iterative on-device testing

Phases with standard patterns (skip research-phase):
- **Phase 1 (Foundation):** Qt data models, JSON serialization, file I/O — extremely well-documented
- **Phase 3 (Sync):** REST API client, offline-first queue, conflict resolution — established patterns with official Todoist docs
- **Phase 4 (Offline Queue):** Command pattern with idempotency — standard distributed systems approach
- **Phase 5 (Task Creation):** Qt forms, virtual keyboard — built-in Qt components

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Qt 5.15 is pre-installed and documented, Todoist REST API v2 is official and stable, Toltec/rm2fb are mature community tools |
| Features | MEDIUM | Feature expectations inferred from e-ink productivity patterns and Todoist capabilities, not validated with reMarkable users |
| Architecture | HIGH | Qt patterns are well-established, offline-first sync is proven architecture, rm2fb integration is documented with template apps |
| Pitfalls | MEDIUM | Critical pitfalls verified from official sources (rm2fb repo, Qt docs, API limits), but reMarkable-specific gotchas may exist beyond research |

**Overall confidence:** MEDIUM-HIGH

Research is strongest for technical implementation (stack, architecture) where official documentation and community resources are abundant. Confidence is moderate for feature prioritization and user expectations, which are inferred from domain patterns rather than direct user research. All critical paths have documented solutions, but device-specific optimizations (e-ink tuning, battery management) will require empirical testing during implementation.

### Gaps to Address

Research limitations and how to handle them:

- **Handwriting OCR quality unknown:** MyScript API accuracy on reMarkable hardware not benchmarked. Handle by deferring to Phase 7 (post-MVP) and building fallback to keyboard input. Consider proof-of-concept with sample strokes before committing to full integration.
- **rm2fb partial update performance:** Optimal refresh frequency for task lists of varying sizes requires on-device testing. Handle by starting conservative (full refresh every 5 partials) and tuning based on actual ghosting during Phase 2 development.
- **Qt Widgets vs Qt Quick:** Documentation says "Qt Quick only" but community reports Widgets work fine. Handle by validating early in Phase 1 with hello-world app that uses both. Pivot to Qt Quick if Widgets fail.
- **Task list size limits:** Performance unknown at 1000+ tasks. Handle by targeting 100-500 tasks for MVP (covers 90% of users), optimize for larger lists in Phase 8 if needed (switch to QAbstractItemModel virtualization or SQLite).
- **OpenSSL availability on device:** Assumed present but not verified. Handle by adding runtime check `QSslSocket::supportsSsl()` in Phase 1, fail gracefully with clear error if missing.
- **Battery impact of sync strategies:** Conservative timer intervals chosen based on community reports, not measured. Handle by implementing telemetry in Phase 6 to track battery usage, adjust defaults based on data.
- **Conflict resolution edge cases:** Field-level merge logic designed based on general offline-first patterns, not Todoist-specific scenarios. Handle by starting with completion-always-wins rule and adding complexity only when real conflicts are discovered during testing.

## Sources

### Primary (HIGH confidence)
- [Todoist REST API v2](https://developer.todoist.com/rest/v2/) — Official API documentation, rate limits, endpoints
- [reMarkable Guide - Qt Framework](https://remarkable.guide/devel/language/c++/qt.html) — Qt 5.15 usage, system integration
- [Qt 5.15 Documentation](https://doc.qt.io/qt-5/) — QNetworkAccessManager, QJsonDocument, threading patterns
- [ddvk/remarkable2-framebuffer](https://github.com/ddvk/remarkable2-framebuffer) — rm2fb compatibility matrix, installation instructions
- [Toltec Package Repository](https://github.com/toltec-dev/toltec) — Distribution standards, opkg packaging
- [Oxide Launcher](https://github.com/Eeems-Org/oxide) — App registration API, desktop integration
- [Touch Target Sizes - NN/g](https://www.nngroup.com/articles/touch-target-size/) — Usability guidelines for touch interfaces

### Secondary (MEDIUM confidence)
- Multiple 2025-2026 technical articles on offline-first architecture and conflict resolution patterns — Consensus on field-level merge strategies
- E-ink display optimization guides (Hackaday, GooDisplay, Viwoods) — Ghosting management and refresh modes
- reMarkable community forums and troubleshooting guides — Battery life expectations, common pitfalls
- Security best practices guides (42Crunch, wolfSSL) — Token storage for embedded devices
- Handwriting recognition benchmarks 2026 (AIMultiple) — OCR accuracy expectations

### Tertiary (LOW confidence, needs validation)
- Community reports on Qt Widgets vs Quick on reMarkable — Conflicting information, needs empirical test
- Performance characteristics with large task lists — No public benchmarks, extrapolated from device specs
- MyScript API integration on reMarkable — Limited documentation for this specific use case

---
*Research completed: 2026-01-29*
*Ready for roadmap: yes*

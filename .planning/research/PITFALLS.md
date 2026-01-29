# Domain Pitfalls

**Domain:** reMarkable 2 Todoist Client (E-ink App + API Sync)
**Researched:** 2026-01-29
**Confidence:** MEDIUM (verified WebSearch + official docs, domain-specific patterns)

## Critical Pitfalls

Mistakes that cause rewrites, bricked devices, or major issues.

### Pitfall 1: Soft-Bricking Device with Unsupported rm2fb Version
**What goes wrong:** Installing rm2fb on an unsupported reMarkable OS version causes the screen to stop displaying anything, making the device unusable (soft-bricked) even though SSH access remains.

**Why it happens:** rm2fb must be updated for each new OS release because function addresses change between firmware versions. The framebuffer is not fully understood, and display access is tightly coupled to specific Xochitl version internals.

**Consequences:**
- Device screen becomes completely non-functional
- Requires SSH access and manual uninstallation to recover
- Users may think device is permanently broken
- Factory reset while Toltec is installed can make this worse

**Prevention:**
- Always check rm2fb config.cpp for your specific Xochitl version before installation
- Install only through Toltec package manager, not manually
- Never factory reset with Toltec/rm2fb installed - uninstall first with `toltecctl uninstall`
- Test on supported OS version before deploying to users
- Document minimum/maximum supported OS versions clearly

**Detection:**
- "Missing address for function [...]" error message indicates unsupported version
- Screen goes black after installation but SSH still works
- Device stuck on "loading" screen after reboot

**Phase Impact:** This must be addressed in Phase 1 (Setup/Infrastructure). Document supported versions in README and add runtime version checks.

**Sources:**
- [GitHub - ddvk/remarkable2-framebuffer](https://github.com/ddvk/remarkable2-framebuffer)
- [Toltec — reMarkable Guide](https://remarkable.guide/guide/software/toltec.html)

---

### Pitfall 2: E-ink Ghosting from Insufficient Full Refreshes
**What goes wrong:** Using only partial/fast updates causes ghosting - faint visual residue of previous content remains visible, making the UI increasingly illegible over time.

**Why it happens:** Partial updates only change pixels that need updating, but microcapsules don't fully realign. Without periodic full refreshes (which flash the screen black-white-target), residue accumulates.

**Consequences:**
- Task list becomes hard to read with ghost text overlays
- Checkboxes appear partially checked when they're not
- User experience degrades significantly after 5-10 interactions
- Users assume the app is buggy or the device is broken

**Prevention:**
- Perform full refresh every 5 consecutive partial updates
- Full refresh when switching between task list and add screen
- Full refresh at least once every 24 hours (for multi-color displays)
- Full refresh when user manually triggers sync
- Make full refresh user-configurable (some users prefer aggressive clearing)
- Consider adding a manual refresh gesture/button

**Detection:**
- Faint previous text visible behind current content
- UI elements look "dirty" or unclear
- User reports app "looks weird" or "text is blurry"

**Phase Impact:** Address in Phase 2 (Basic Display). Build refresh management into display layer from the start.

**Sources:**
- [E Ink Ghosting Decoded - Viwoods](https://viwoods.com/blogs/paper-tablet/e-ink-ghosting-explained)
- [E-paper basics 1: Update modes - Hackaday](https://hackaday.io/project/21551-paperino/log/59392-e-paper-basics-1-update-modes)
- [GooDisplay ePaper Usage Guidelines](https://www.good-display.com/news/80.html)

---

### Pitfall 3: Last-Write-Wins Conflict Resolution Causing Silent Data Loss
**What goes wrong:** When the same task is modified offline on multiple devices (or modified locally while server changes happen), simple "last write wins" (LWW) overwrites earlier changes, silently losing user edits.

**Why it happens:** Developers assume LWW is "good enough" for simple apps. Clock skew between device and server makes LWW non-deterministic. The ease of implementation masks the severity of data loss.

**Consequences:**
- User completes a task offline, someone else edits it, sync overwrites the completion
- User adds priority/due date offline, server has different changes, one gets lost
- No error message - data just vanishes
- User loses trust in app ("I know I marked that done!")
- Particularly bad for task completion status

**Prevention:**
- Never use whole-document overwrites - use field-level updates
- For task completion: completion always wins over other field edits
- Track operation order with monotonic IDs, not timestamps
- Implement operation log/queue with causality tracking
- Use Todoist's sync token system correctly for incremental sync
- Show user when conflicts occur - don't hide them
- For non-reconcilable conflicts, prefer local changes (user's device = source of truth)
- Add conflict resolution tests to sync logic

**Detection:**
- User reports "my changes disappeared"
- Completion status randomly reverts
- Multiple devices show different task states
- Sync seems to "undo" recent changes

**Phase Impact:** Must be designed correctly in Phase 3 (Sync Architecture). Retrofitting conflict resolution is extremely difficult.

**Sources:**
- [Offline-First Architecture - Medium](https://medium.com/@jusuftopic/offline-first-architecture-designing-for-reality-not-just-the-cloud-e5fd18e50a79)
- [Building Offline-First Mobile Sync - DEV](https://dev.to/omaranajar/building-offline-first-mobile-sync-lessons-from-the-trenches-3m21)
- [Beyond Timestamps: Advanced Conflict Resolution - Medium](https://medium.com/@pranavdixit20/beyond-timestamps-advanced-conflict-resolution-in-offline-first-django-apps-ii-5e3d9f36541b)

---

### Pitfall 4: Hardcoded API Token in Source Code
**What goes wrong:** Developer embeds Todoist API token directly in code or config files, which gets committed to GitHub, making it easy for attackers to extract and abuse.

**Why it happens:** It's convenient during development. Developers don't realize embedded secrets are easily extracted from binaries. Lack of secure storage knowledge for embedded devices.

**Consequences:**
- Anyone can extract token from app binary or repository
- Attacker gains full access to user's Todoist account
- User data compromised, tasks deleted/modified by malicious actors
- Violates OAuth best practices
- Potential legal liability for leaked user data
- App gets pulled from Toltec if discovered

**Prevention:**
- Never hardcode tokens in source code, config files, or firmware
- Use file-based storage with restricted permissions (chmod 600)
- Store tokens in ~/.config/remarkable-todoist/credentials (not in /opt with app)
- On first run, prompt user for API token and save securely
- Consider Qt's QSettings with encryption for credential storage
- Document in README that users must provide their own API token
- Add .gitignore entries for any credential files
- Use environment variables during development (never commit .env)

**Detection:**
- `grep -r "api.*token" src/` finds hardcoded strings
- Binary strings analysis reveals embedded secrets
- Code review finds configuration with credentials

**Phase Impact:** Must be addressed in Phase 1 (Project Setup). Pattern must be established before any API integration work.

**Sources:**
- [API Security Best Practices - 42Crunch](https://42crunch.com/token-management-best-practices/)
- [Secure Token Storage - Capgo](https://capgo.app/blog/secure-token-storage-best-practices-for-mobile-developers/)
- [Best Practices for Embedded Devices - wolfSSL](https://www.wolfssl.com/documentation/manuals/wolfssl/chapter12.html)

---

## Moderate Pitfalls

Mistakes that cause delays, technical debt, or degraded UX.

### Pitfall 5: No Exponential Backoff for API Rate Limits
**What goes wrong:** App hits Todoist's rate limit (450 requests per 15 minutes), retries immediately, gets rejected again, drains battery with retry loop, and leaves user unable to sync.

**Why it happens:** Developer implements simple retry logic without backoff. Not testing with realistic sync load (many tasks). Assuming rate limits won't be hit.

**Consequences:**
- Sync fails repeatedly, user can't get latest tasks
- Battery drain from continuous retry attempts
- 15-minute lockout from API even for legitimate requests
- Background sync service keeps device awake
- User assumes app is broken

**Prevention:**
- Implement exponential backoff with jitter: `delay = baseDelay * (2^retryCount) + random_jitter`
- Start with 1s delay, max out at 60s
- Add jitter (±500ms random) to prevent thundering herd if multiple users hit limit
- Limit retry attempts (max 5 retries before giving up)
- Only retry on 429 (rate limit) and 5xx (server error), not 4xx (client error)
- Check for `Retry-After` header and respect it
- Log rate limit hits to monitor if app is too aggressive
- Consider request batching to reduce API calls

**Detection:**
- Logs show repeated 429 responses
- Battery drains faster than expected
- User reports sync "spinning forever"
- API usage spikes in monitoring

**Phase Impact:** Must be implemented in Phase 3 (Sync Integration). Retrofit adds complexity.

**Sources:**
- [Complete Guide to Handling API Rate Limits - Ayrshare](https://www.ayrshare.com/complete-guide-to-handling-rate-limits-prevent-429-errors/)
- [Mastering Exponential Backoff - Better Stack](https://betterstack.com/community/guides/monitoring/exponential-backoff/)
- [Todoist REST API Reference](https://developer.todoist.com/rest/v1/)

---

### Pitfall 6: Touch Targets Too Small for Stylus/Finger Use
**What goes wrong:** Checkboxes and interactive elements are too small, causing users to miss targets or accidentally tap wrong items, especially frustrating on e-ink with no visual feedback during press.

**Why it happens:** Copying desktop checkbox sizes (24×24px) to embedded device. Not accounting for stylus tip size or finger use. Not testing on actual hardware with stylus.

**Consequences:**
- User taps checkbox, nothing happens (missed target)
- User accidentally checks wrong task
- Rage taps - repeated frustrated tapping
- Worse on e-ink because no immediate visual feedback confirms tap
- Accessibility issues for users with motor impairments
- Users abandon app as "too hard to use"

**Prevention:**
- Minimum touch target: 48×48dp (9mm physical) for all interactive elements
- Larger at screen edges: 11mm top, 12mm bottom, 7mm center
- Make entire task row tappable, not just checkbox
- Add padding/margin between checkboxes (min 8dp spacing)
- Consider 64×64dp targets for primary actions
- Label-associated click areas (tap task name to toggle)
- Test with reMarkable stylus AND finger on actual device
- Visual indicator on hover/press even with e-ink delay

**Detection:**
- User testing reveals missed taps
- Analytics show high tap counts with low action rates
- User feedback: "hard to click" or "keeps missing"

**Phase Impact:** Design in Phase 2 (UI Design). Fixing later requires full UI rework.

**Sources:**
- [Touch Targets on Touchscreens - NN/g](https://www.nngroup.com/articles/touch-target-size/)
- [Accessible Target Sizes Cheatsheet - Smashing Magazine](https://www.smashingmagazine.com/2023/04/accessible-tap-target-sizes-rage-taps-clicks/)
- [Designing better target sizes - Ahmad Shadeed](https://ishadeed.com/article/target-size/)

---

### Pitfall 7: Handwriting Recognition Assuming Perfect OCR
**What goes wrong:** App treats OCR output as perfectly accurate, doesn't allow correction, submits gibberish task names to Todoist, frustrating users.

**Why it happens:** Developer tests with own neat handwriting. Not testing with rushed writing, cursive, or diverse writing styles. Assuming 2026 OCR is "good enough."

**Consequences:**
- Tasks created with wrong names: "Buy milk" becomes "Buy mdlk"
- No way to fix errors before submission
- User must go to phone to rename tasks
- Defeats purpose of reMarkable workflow
- User stops using handwriting feature
- OCR errors compound with poor image quality, tilt, or connected characters

**Prevention:**
- Show OCR output in editable text field BEFORE creating task
- Add "Edit" button next to recognized text
- Provide on-screen keyboard for corrections (Qt virtual keyboard)
- Save both handwriting image and OCR text
- Allow re-running OCR with different confidence threshold
- Test with diverse handwriting samples (cursive, print, messy)
- Document OCR limitations in UI ("Review before submitting")
- Consider confidence scores - flag low-confidence words for review
- Proper image preprocessing: deskew, denoise, normalize contrast

**Detection:**
- User reports "wrong task names"
- Tasks in Todoist have obvious OCR errors
- User stops using handwriting feature
- Support requests about "fixing task names"

**Phase Impact:** Design in Phase 4 (Handwriting). Changing UX flow later is disruptive.

**Sources:**
- [Handwriting Recognition Benchmark - AIMultiple 2026](https://research.aimultiple.com/handwriting-recognition/)
- [9 Biggest OCR Limitations - DocuClipper](https://www.docuclipper.com/blog/ocr-limitations/)
- [State of OCR in 2026 - AIMultiple](https://research.aimultiple.com/ocr-technology/)

---

### Pitfall 8: Background Sync Service Draining Battery
**What goes wrong:** Auto-sync service keeps Wi-Fi active, wakes device frequently, drains battery from days to hours, ruining reMarkable's best feature.

**Why it happens:** Porting mobile app patterns to e-ink device. Not understanding reMarkable's sleep/wake cycles. Sync interval too aggressive. Leaving Wi-Fi on all the time.

**Consequences:**
- Battery life drops from 2 weeks to 2 days
- Device stays warm from activity
- User gets "low battery" warnings constantly
- Defeats reMarkable's paper-like always-on benefit
- User disables auto-sync, defeating app purpose
- Negative reviews citing battery drain

**Prevention:**
- Default to manual sync, not auto-sync
- If auto-sync enabled, use conservative intervals (15-30 minutes minimum)
- Turn Wi-Fi off after each sync completes (don't leave on)
- Don't wake device from sleep for background sync
- Only sync when device is actively in use
- Use systemd timer instead of long-running daemon
- Provide battery-saving mode: sync only on user action + manual refresh
- Monitor battery impact during development
- Document battery trade-offs in settings

**Detection:**
- Battery drains faster than expected (hours instead of days)
- `top` shows sync service constantly running
- Wi-Fi shows as always active
- Device doesn't enter deep sleep
- User complaints about battery life

**Phase Impact:** Architecture decision in Phase 3 (Sync). Changing later requires service redesign.

**Sources:**
- [reMarkable Battery Saving Tips - eReadersForum](https://www.ereadersforum.com/threads/remarkable-battery-saving-tips-extend-life-on-rm1-rm2-paper-pro.8209/)
- [10 Common reMarkable Problems - TabletSage](https://tabletsage.com/remarkable-problems-solutions/)
- [Battery issues - reMarkable Support](https://support.remarkable.com/s/article/Battery-issues)

---

### Pitfall 9: Not Handling Todoist API Payload Limits
**What goes wrong:** Syncing large task lists (1000+ tasks) or tasks with long notes hits Todoist's 1 MiB payload limit, request fails, sync never completes.

**Why it happens:** Not testing with realistic data volumes. Assuming "sync all" works at any size. Not reading API documentation carefully.

**Consequences:**
- Initial sync fails for power users
- Error message unclear ("request too large")
- App unusable for users with many tasks
- Partial sync leaves device in inconsistent state
- User can't complete onboarding

**Prevention:**
- Implement pagination for initial sync
- Fetch tasks in batches of 100-200 per request
- Use incremental sync after initial load (Todoist sync token)
- Add progress indicator for multi-page syncs
- Filter to specific projects if user has 5000+ tasks
- Handle 413 Payload Too Large gracefully with retry at smaller batch size
- Test with accounts containing 1000+ tasks
- Monitor payload size before sending (warn if approaching 1 MiB)
- Document task limit in README if necessary

**Detection:**
- 413 or 400 responses during sync
- Sync fails with "request too large" error
- Works for small task lists but fails for large ones
- Logs show requests >1 MiB

**Phase Impact:** Must be designed into Phase 3 (Sync Architecture). Pagination retrofit is complex.

**Sources:**
- [Todoist REST API Reference](https://developer.todoist.com/rest/v1/)
- [Todoist Sync API Reference](https://developer.todoist.com/sync/v9/)

---

## Minor Pitfalls

Mistakes that cause annoyance but are fixable.

### Pitfall 10: Slow E-ink Refresh Blocking UI Thread
**What goes wrong:** Performing display updates on main Qt thread causes entire app to freeze for 1-2 seconds during refresh, making app feel unresponsive.

**Why it happens:** Not understanding e-ink refresh is slow (10-15 seconds for full, 1-2s for partial). Using synchronous display updates. Porting from fast LCD assumptions.

**Consequences:**
- App appears frozen during task check/uncheck
- No feedback that tap was registered
- User taps multiple times thinking app crashed
- Poor UX even though functionality works

**Prevention:**
- Move display updates to background thread (QThread)
- Show immediate optimistic UI update while actual refresh happens
- Use Qt's event loop correctly - don't block main thread
- Add loading indicator or progress message during slow operations
- Consider queuing rapid updates and batching
- Test on actual reMarkable hardware (emulator is too fast)

**Detection:**
- App freezes during interactions
- CPU shows high usage but app is "doing nothing"
- User reports "app hangs when I tap"

**Phase Impact:** Can be fixed in any phase but easier to prevent in Phase 2 (Display Implementation).

---

### Pitfall 11: Memory Leaks from Qt Widget Creation/Deletion
**What goes wrong:** Creating and deleting dialogs/widgets repeatedly causes memory fragmentation and eventual crashes on resource-constrained reMarkable.

**Why it happens:** Not following Qt parent-child memory management. Creating new widgets instead of reusing. Not understanding Qt's automatic deletion via parent relationship.

**Consequences:**
- Memory usage grows over time
- App crashes after extended use
- Slower performance as memory fragments
- reMarkable's limited RAM exhausted

**Prevention:**
- Create widgets once, then use hide()/show() instead of delete/new
- Properly parent all widgets (Qt deletes children with parent)
- Use RAII patterns and smart pointers (QPointer, QSharedPointer)
- Monitor memory usage during development with valgrind
- Run long-duration stress tests (24+ hours)
- Use Qt Resource System to reduce memory pressure
- Only include needed Qt modules in build

**Detection:**
- Memory usage grows over time (`top` shows increasing RSS)
- Valgrind reports leaks
- Crashes after hours of use
- Slower performance over time

**Phase Impact:** Establish patterns in Phase 1 (Setup). Fixing leaks later requires full audit.

**Sources:**
- [C++ for Embedded - Qt](https://qt.io/embedded-development-talk/c-for-embedded-advantages-disadvantages-and-myths)
- [Memory Requirement for Qt - Qt Forum](https://forum.qt.io/topic/51062/memory-requirement-for-qt)
- [Optimizing Qt Application Performance - DevGlan](https://www.devglan.com/guest-post/optimizing-QT-application-performance-in-embedded-systems)

---

### Pitfall 12: Not Designing for E-ink's Monochrome/Grayscale Constraints
**What goes wrong:** UI uses subtle color cues (red for urgent, gray for disabled) which are invisible on monochrome e-ink, making features unusable.

**Why it happens:** Designing on color LCD during development. Not testing on actual e-ink display. Assuming grayscale is "good enough" for color designs.

**Consequences:**
- Priority indicators invisible
- Disabled UI elements look enabled
- No visual distinction between states
- Accessibility issues
- User can't tell what's interactive

**Prevention:**
- Design with grayscale + patterns/icons, not color alone
- Use bold/regular weight, size, icons, borders for state differences
- Test on actual reMarkable hardware, not emulator
- High contrast between text and background (black on white)
- Patterns for priority: P1=bold, P2=normal, P3=light + icon
- Visual indicators for due dates: overdue=strikethrough + icon
- Never rely solely on color (also violates accessibility guidelines)

**Detection:**
- Can't distinguish priority levels on device
- Disabled buttons look clickable
- UI looks "flat" or "all the same"
- User reports "can't tell what's what"

**Phase Impact:** Design principle for Phase 2 (UI Design). Fixing later requires UI redesign.

**Sources:**
- [E-Ink Display Integration - Core Electronics](https://forum.core-electronics.com.au/t/e-ink-display-integration-ghosting-and-refresh-challenges/23151)
- [Is an E-Ink Display Right for Your Next Project - DigiKey](https://www.digikey.com/en/maker/blogs/2022/is-an-e-ink-display-right-for-your-next-project)

---

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|---------------|------------|
| Phase 1: Project Setup | Hardcoding API tokens | Establish secure storage pattern immediately |
| Phase 1: Environment Setup | Installing on unsupported Xochitl version | Document version compatibility, add runtime checks |
| Phase 2: Display Layer | Ignoring ghosting management | Build refresh strategy into display abstraction |
| Phase 2: UI Design | Touch targets too small | Use 48×48dp minimum, test with stylus |
| Phase 2: UI Design | Color-dependent UI | Design for grayscale+patterns from start |
| Phase 3: Sync Architecture | Last-write-wins conflict resolution | Design field-level merge strategy upfront |
| Phase 3: API Integration | No rate limit handling | Implement exponential backoff from start |
| Phase 3: Sync Service | Background sync draining battery | Default to manual sync, conservative timers |
| Phase 4: Handwriting | Assuming perfect OCR | Show editable OCR output before submission |
| Phase 5: Offline Queue | Not handling failed request accumulation | Implement queue size limits and pruning |

---

## Research Methodology & Confidence

**Overall Confidence: MEDIUM**

### High Confidence (Verified with Official Sources):
- rm2fb version compatibility issues (GitHub repo + community documentation)
- E-ink refresh patterns and ghosting (manufacturer documentation + hardware guides)
- Todoist API rate limits and constraints (official API documentation)
- Touch target size guidelines (WCAG + platform guidelines)
- Qt memory management for embedded (official Qt documentation + forums)

### Medium Confidence (Multiple WebSearch Sources Agree):
- Offline sync conflict resolution patterns (recent 2025-2026 technical articles)
- Battery drain from background services (community reports + troubleshooting guides)
- Handwriting OCR accuracy challenges (2026 benchmarks + industry reports)
- Toltec installation pitfalls (community guide + GitHub issues)

### Low Confidence / Needs Validation:
- Specific rm2fb performance characteristics (limited recent documentation)
- Exact Qt 5.15 + e-ink integration issues (no specific reports found)
- reMarkable-specific Qt optimization requirements (general embedded principles applied)

### Known Gaps:
- No official reMarkable app development postmortem documentation found
- Limited 2026-specific rm2fb issue reports (project may be stable or low activity)
- Todoist Sync API v9 is deprecated; conflict resolution best practices not explicitly documented
- No public case studies of reMarkable + REST API sync implementations

### Verification Approach:
1. Official documentation prioritized (Todoist API docs, Qt docs, WCAG)
2. Technical implementation guides cross-referenced (multiple 2025-2026 articles on offline sync)
3. Community patterns identified (GitHub issues, forums, troubleshooting guides)
4. General principles applied where specific docs unavailable (embedded best practices → reMarkable context)

---

## Sources

### Official Documentation:
- [Todoist REST API Reference v1](https://developer.todoist.com/rest/v1/)
- [Todoist Sync API Reference v9](https://developer.todoist.com/sync/v9/)
- [reMarkable Developer Documentation](https://developer.remarkable.com/documentation)
- [reMarkable Guide - Development](https://remarkable.guide/devel/index.html)

### Project Repositories:
- [GitHub - ddvk/remarkable2-framebuffer](https://github.com/ddvk/remarkable2-framebuffer)
- [GitHub - toltec-dev/toltec](https://github.com/toltec-dev/toltec)
- [reMarkable Wiki - The reMarkable 2 Framebuffer](https://remarkablewiki.com/tech/rm2_framebuffer)

### Technical Articles (2025-2026):
- [Offline-First Architecture: Designing for Reality - Medium](https://medium.com/@jusuftopic/offline-first-architecture-designing-for-reality-not-just-the-cloud-e5fd18e50a79)
- [Building Offline-First Mobile Sync - DEV](https://dev.to/omaranajar/building-offline-first-mobile-sync-lessons-from-the-trenches-3m21)
- [Beyond Timestamps: Advanced Conflict Resolution - Medium](https://medium.com/@pranavdixit20/beyond-timestamps-advanced-conflict-resolution-in-offline-first-django-apps-ii-5e3d9f36541b)
- [Offline-first frontend apps in 2025 - LogRocket](https://blog.logrocket.com/offline-first-frontend-apps-2025-indexeddb-sqlite/)
- [Handwriting Recognition Benchmark 2026 - AIMultiple](https://research.aimultiple.com/handwriting-recognition/)
- [State of OCR in 2026 - AIMultiple](https://research.aimultiple.com/ocr-technology/)

### Hardware & Display:
- [E Ink Ghosting Decoded - Viwoods](https://viwoods.com/blogs/paper-tablet/e-ink-ghosting-explained)
- [E-paper basics 1: Update modes - Hackaday](https://hackaday.io/project/21551-paperino/log/59392-e-paper-basics-1-update-modes)
- [ePaper Display Usage Guidelines - GooDisplay](https://www.good-display.com/news/80.html)
- [How to Optimize Ghosting on Color E Ink Screen - BOOX](https://onyxboox.medium.com/how-to-optimize-ghosting-on-color-e-ink-screen-fa0b9b77a171)

### Security & Best Practices:
- [API Security Best Practices - 42Crunch](https://42crunch.com/token-management-best-practices/)
- [Secure Token Storage - Capgo](https://capgo.app/blog/secure-token-storage-best-practices-for-mobile-developers/)
- [Best Practices for Embedded Devices - wolfSSL](https://www.wolfssl.com/documentation/manuals/wolfssl/chapter12.html)
- [Complete Guide to Handling API Rate Limits - Ayrshare](https://www.ayrshare.com/complete-guide-to-handling-rate-limits-prevent-429-errors/)
- [Mastering Exponential Backoff - Better Stack](https://betterstack.com/community/guides/monitoring/exponential-backoff/)

### Qt & Embedded Development:
- [C++ for Embedded: Advantages, Disadvantages, and Myths - Qt](https://qt.io/embedded-development-talk/c-for-embedded-advantages-disadvantages-and-myths)
- [Optimizing Qt Application Performance - DevGlan](https://www.devglan.com/guest-post/optimizing-QT-application-performance-in-embedded-systems)
- [Qt for Embedded Linux - Qt 5.15](https://doc.qt.io/archives/qt-5.15/embedded-linux.html)

### Usability & Accessibility:
- [Touch Targets on Touchscreens - NN/g](https://www.nngroup.com/articles/touch-target-size/)
- [Accessible Target Sizes Cheatsheet - Smashing Magazine](https://www.smashingmagazine.com/2023/04/accessible-tap-target-sizes-rage-taps-clicks/)
- [Designing better target sizes - Ahmad Shadeed](https://ishadeed.com/article/target-size/)

### Community Resources:
- [Toltec — reMarkable Guide](https://remarkable.guide/guide/software/toltec.html)
- [reMarkable Battery Saving Tips - eReadersForum](https://www.ereadersforum.com/threads/remarkable-battery-saving-tips-extend-life-on-rm1-rm2-paper-pro.8209/)
- [10 Common reMarkable Problems - TabletSage](https://tabletsage.com/remarkable-problems-solutions/)
- [Battery issues - reMarkable Support](https://support.remarkable.com/s/article/Battery-issues)

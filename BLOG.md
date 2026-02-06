# Building a Todoist Client for reMarkable: A Journey in AI-Assisted Embedded Development

*How Claude and I built a Qt6/QML app for e-ink, solved cross-compilation mysteries, and created a launcher that watches for magical notebooks*

---

## The Challenge

I wanted my Todoist tasks on my reMarkable 2 tablet. Not through a browser, not synced to images—a native app that felt right on e-ink. The catch? The reMarkable runs stock firmware with Qt6, uses an e-paper display backend, and requires ARM32 cross-compilation from my arm64 Linux machine. Oh, and I wanted to leverage AI assistance to move fast.

This is the story of building that app using Claude and the GSD (Get Stuff Done) methodology, discovering Qt6's quirks on e-ink hardware, and creating a launcher system that turns opening a notebook into launching an app.

## Part 1: AI-Assisted Development with GSD

### The GSD Approach

GSD (Get Stuff Done) is a structured AI-assisted development workflow that breaks projects into phases, plans, and atomic commits. Think of it as having an AI pair programmer that doesn't just write code—it architects solutions, maintains context across sessions, and ships features methodically.

Here's what a GSD roadmap looks like:

```markdown
Phase 1: Foundation & Task Display (COMPLETE ✅)
  - 4 plans executed in 3 waves
  - Goal: Display all Todoist tasks on device
  - Result: Scrollable task list with name, due date, project, priority

Phase 2: Sync & Task Completion (COMPLETE ✅)
  - 4 plans with offline queue, sync manager
  - Goal: Mark tasks complete, handle offline mode
  - Result: Resilient sync with queued operations

Phase 3: Task Creation (IN PROGRESS 🚧)
  - OCR integration, drawing canvas
  - Goal: Add tasks via handwriting
  - Challenge: Tesseract not on device (yet)
```

Each phase has clear success criteria. Each plan is a single focused implementation. Each commit is atomic. This structure lets Claude maintain context across days and pick up exactly where we left off.

### What AI Assistance Actually Looks Like

Let me show you a real example. When we hit cross-compilation linker errors, here's what happened:

**The Problem:**
```bash
/usr/bin/ld: cannot find -lQt6Core: No such file or directory
/usr/bin/ld: cannot find -lQt6Gui: No such file or directory
/usr/bin/ld: cannot find -licuuc: No such file or directory
```

**The AI's Debugging Process:**

1. **Identified the root cause**: The linker needs `.so` symlinks, but we only pulled `.so.6` and `.so.6.8.2` files from the device.

2. **Proposed a solution**: Create a script that automatically creates the symlinks:
   ```bash
   # Create .so -> .so.6 symlinks for linker
   for lib in *.so.*; do
       base=$(echo "$lib" | sed 's/\.so\..*/\.so/')
       ln -s "$lib" "$base"
   done
   ```

3. **Went further**: Built the fix into the build system so it happens automatically, not manually.

4. **Added graceful degradation**: Made OCR libraries optional with conditional compilation.

This is the power of AI assistance—it doesn't just fix the immediate problem, it considers the bigger picture: "What if someone else builds this? What if the libraries change? How do we make this robust?"

### The GSD Workflow in Practice

```bash
# Phase planning
/gsd:plan-phase 3

# AI researches Qt6 on reMarkable, Tesseract integration
# Creates detailed PLAN.md files
# Identifies dependencies and risks

# Execution
/gsd:execute-phase 3

# AI implements each plan atomically
# Runs tests, verifies on device
# Creates commits with clear messages
```

Each plan gets its own research document, implementation strategy, and verification steps. When issues arise (and they always do), the AI debugs systematically, checking device capabilities, testing hypotheses, and documenting learnings.

## Part 2: Qt6 on reMarkable—The Good, The Weird, The E-Ink

### The reMarkable Runtime Environment

The reMarkable 2 runs stock firmware built on Qt6—not Qt5 Widgets, but Qt6 Quick/QML. This is both blessing and curse.

**The blessing:** Modern declarative UI, hardware-accelerated (for e-ink), and QML works well for touch interfaces.

**The curse:** It's *Qt6 on e-ink hardware with custom backends*. Here's what that means:

```bash
# Required environment variables to run anything
export QT_QPA_PLATFORM=epaper         # Use e-paper backend
export QT_QUICK_BACKEND=epaper        # QML also needs to know
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="rotate=180:invertx"
export HOME=/home/root                # Critical for config loading
```

Miss any of these, and your app crashes silently, renders to the wrong framebuffer, or flips the touch coordinates 180 degrees.

### Cross-Compilation: The Hybrid Approach

Here's where it gets interesting. We can't use the device's compiler (too slow, limited space), and a full cross-compilation SDK is overkill. The solution? A hybrid approach:

```bash
# On the host (arm64 Linux):
MOC=/usr/lib/qt6/libexec/moc          # Host Qt tools
RCC=/usr/lib/qt6/libexec/rcc          # (architecture-independent)
QT6_INC=/usr/include/aarch64-linux-gnu/qt6  # Host headers

# For the target (ARM32):
CXX=arm-linux-gnueabihf-g++           # Cross-compiler
SYSROOT=/tmp/rm-sysroot               # Device libraries
```

**Why this works:**

1. **Qt tools (moc, rcc)** generate C++ code—architecture-independent
2. **Qt headers** describe APIs—architecture-independent  
3. **Device libraries** contain the actual ARM32 code—architecture-specific

We run host tools, use host headers, compile for ARM32, and link against device libraries. It's like using a map of a city (headers) and a local guide (libraries) to navigate somewhere you've never been.

### The Sysroot Dance

Pulling libraries from the device is straightforward:

```bash
ssh root@10.11.99.1 "tar czf - /usr/lib/*.so* /lib/*.so*" \
    | tar xzf - -C /tmp/rm-sysroot/
```

But here's the gotcha: you get files like `libQt6Core.so.6.8.2` and `libQt6Core.so.6`, but the linker wants `libQt6Core.so`. You need symlinks:

```bash
libQt6Core.so -> libQt6Core.so.6 -> libQt6Core.so.6.8.2
```

Our build script now does this automatically. It even detects optional libraries (like Tesseract for OCR) and conditionally compiles features:

```cpp
#ifdef ENABLE_OCR
    m_recognizer = new HandwritingRecognizer(this);
    if (!m_recognizer->initialize()) {
        qWarning() << "OCR initialization failed";
    }
#else
    qDebug() << "OCR support not compiled";
#endif
```

The app builds and runs whether or not OCR libraries are present. This is *graceful degradation*, and it's critical for embedded systems where you can't control the runtime environment.

### QML for E-Ink: Lessons Learned

**1. Partial updates are your friend**

E-ink has ghosting. Full screen refreshes are slow. QML's declarative updates help:

```qml
ListView {
    id: taskListView
    model: taskModel
    delegate: TaskDelegate {
        // Only this item updates when task changes
        onCheckedChanged: taskModel.setTaskCompleted(index, checked)
    }
}
```

When you check off a task, only that checkbox region updates. Fast and ghost-free.

**2. Touch targets matter more on e-ink**

Capacitive touch on e-ink feels different than on LCD. We made touch areas generous:

```qml
CheckBox {
    width: 60   // Minimum 60px for stylus comfort
    height: 60
}
```

**3. Black and white only**

No gradients, no transparency tricks. Design for 1-bit color:

```qml
Rectangle {
    color: checked ? "#000000" : "#FFFFFF"
    border.color: "#000000"
    border.width: 2
}
```

High contrast, clear borders, no subtlety.

## Part 3: The Launcher System—Notebooks as Triggers

### The Challenge

Stock reMarkable firmware doesn't have an app launcher. You can't just "install an app" from a menu. The options are:

1. **SSH in and run manually** (annoying)
2. **Install Toltec/Oxide** (requires modding)
3. **Create a launch trigger** (clever)

We went with option 3 and built something unique.

### How It Works

The launcher watches for a specific notebook name. When you create and open a notebook called "Launch Todoist", the app launches. When you exit the app, you're back in the notebook. It's like using a notebook as a launcher icon.

**The systemd service:**

```bash
[Unit]
Description=Todoist Launcher
After=xochitl.service

[Service]
Type=simple
ExecStart=/opt/bin/todoist-launcher.sh
Restart=always
User=root

[Install]
WantedBy=multi-user.target
```

**The launcher script:**

```bash
#!/bin/bash

TRIGGER_NOTEBOOK="Launch Todoist"
XOCHITL_DIR="/home/root/.local/share/remarkable/xochitl"
APP_BIN="/opt/bin/remarkable-todoist"

# Watch for the trigger notebook to be opened
inotifywait -m -e open "$XOCHITL_DIR" | while read path action file; do
    # Check if the opened file is our trigger
    notebook_name=$(grep -l "\"visibleName\":\"$TRIGGER_NOTEBOOK\"" \
                    "$XOCHITL_DIR"/*.metadata 2>/dev/null | head -1)
    
    if [ -n "$notebook_name" ]; then
        # Stop the UI
        systemctl stop xochitl
        
        # Launch our app
        export HOME=/home/root
        export QT_QPA_PLATFORM=epaper
        export QT_QUICK_BACKEND=epaper
        export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="rotate=180:invertx"
        
        $APP_BIN
        
        # Restart the UI
        systemctl start xochitl
    fi
done
```

**What's happening:**

1. `inotifywait` monitors the xochitl (reMarkable UI) directory
2. When a file is opened, we check if it's the trigger notebook
3. If yes, stop xochitl (frees the framebuffer)
4. Set up environment variables
5. Launch our app
6. When app exits, restart xochitl

The user experience is magical: create a notebook, name it "Launch Todoist", open it, and suddenly you're in the Todoist app. Close the app, and you're back in the notebook.

### Why This Works

The reMarkable stores notebooks as metadata files:

```json
{
  "visibleName": "Launch Todoist",
  "type": "DocumentType",
  "lastModified": "1738876543210"
}
```

We search for this name pattern and use the file open event as our trigger. It's non-invasive (doesn't modify system files), survives firmware updates, and feels native.

### The Tradeoffs

**Pros:**
- No system modification required
- Works on stock firmware
- Survives updates
- Intuitive UX

**Cons:**
- Requires running daemon (uses ~1MB RAM)
- Only works for one app (but could be extended)
- Slight delay on launch (~1 second)

For a personal app, the tradeoffs are worth it.

## Part 4: Technical Learnings & War Stories

### 1. The Mystery of the Missing Libraries

When building, we got:

```
cannot find -ltesseract
cannot find -llept
```

These are OCR libraries. They're not on stock firmware. We could:
- Build them statically (large binary, complex dependencies)
- Require Toltec (barrier to entry)
- Make them optional (pragmatic)

We chose option 3. The app now builds with or without OCR:

```bash
# In build script
if [ -e "$SYSROOT/usr/lib/libtesseract.so" ]; then
    OCR_AVAILABLE=true
    CXXFLAGS="$CXXFLAGS -DENABLE_OCR"
    LIBS="$LIBS -ltesseract -llept"
fi
```

**Lesson:** Design for the environment you have, not the one you wish you had. Make features optional. Ship what works.

### 2. The Sync Queue Pattern

Offline-first sync is hard. Here's our approach:

```cpp
// User marks task complete
void completeTask(QString taskId) {
    // Optimistic update (immediate UI feedback)
    m_taskModel->markCompleted(taskId);
    
    // Queue the operation
    m_syncQueue->enqueue({
        .type = "close_task",
        .taskId = taskId,
        .timestamp = QDateTime::currentDateTime()
    });
    
    // Try to sync (fails silently if offline)
    m_syncManager->sync();
}
```

The sync manager watches for network changes:

```cpp
connect(QNetworkInformation::instance(), 
        &QNetworkInformation::reachabilityChanged,
        this, &SyncManager::onNetworkChanged);
```

When WiFi comes back, queued operations flush automatically. The queue persists to disk (JSON file), so even device reboots don't lose data.

**Lesson:** Treat network as unreliable. Optimistic updates + persistent queue = good UX on spotty connections.

### 3. The GCC 9.1 ABI Warning Rabbit Hole

Every compilation spewed warnings:

```
parameter passing for argument of type 'const QJsonValueConstRef' 
changed in GCC 9.1
```

These look scary but are informational—the host compiler (GCC 14) is warning about ABI changes since GCC 9.1. Since the target device uses a compatible GCC version, we're fine. 

We could silence them with `-Wno-psabi`, but I left them visible—they're a good reminder that we're doing cross-compilation magic and should test thoroughly on device.

**Lesson:** Not all compiler output requires action. Learn to distinguish signal from noise.

### 4. The HOME Variable That Broke Everything

This one was subtle. App worked via SSH but crashed from launcher. The difference?

```bash
# SSH session
HOME=/home/root          # Set by SSH

# Launcher context  
HOME=/root               # Default systemd value
```

Qt looks for config files at `$HOME/.config/remarkable-todoist/`. When HOME was wrong, config loading failed silently, no API token found, app crashed.

The fix:

```bash
export HOME=/home/root   # In launcher script
```

**Lesson:** Environment variables matter. Test in the actual deployment context, not just your dev environment.

## Part 5: The AI Collaboration Experience

### What Works Well

**1. Systematic Debugging**

When issues arose, Claude didn't guess—it gathered data:

```
> Check if device is reachable: ping 10.11.99.1
> List sysroot contents: ls /tmp/rm-sysroot/usr/lib
> Verify symlinks: ls -l libQt6Core.so
> Test minimal build: compile single file
```

This methodical approach found root causes fast.

**2. Context Maintenance**

With GSD, context persists across sessions. I could say "the OCR issue from yesterday" and Claude knew exactly what I meant—it had access to the full conversation history and project structure.

**3. Proactive Documentation**

Claude didn't just fix issues—it documented them. The BUILD.md file, troubleshooting guide, and inline comments are all AI-generated and actually useful.

### What Required Human Judgment

**1. Architecture Decisions**

"Should we require Toltec or make OCR optional?" This is a product decision, not a technical one. AI can present tradeoffs, but humans choose direction.

**2. UX Sensibilities**

The launcher notebook approach came from me. AI implemented it beautifully, but the creative "use a notebook as a launcher" concept needed human intuition.

**3. Acceptance Criteria**

"Is the ghosting acceptable?" "Does the sync feel fast enough?" These subjective quality judgments require human testing on real hardware.

### The Partnership Model

Think of AI as a senior engineer with perfect recall but no ego:

- **Me:** "The build fails with linking errors"
- **AI:** "Let me check the sysroot... ah, symlinks are missing. Here's a script to fix it. Should I also add automatic detection?"
- **Me:** "Yes, and make it part of the build process"
- **AI:** *implements, tests, documents, commits*

It's collaborative. I set direction, AI executes with thoroughness I couldn't sustain manually.

## Part 6: What's Next

The app works! Phases 1-2 are complete and deployed. You can:
- View all Todoist tasks on reMarkable
- Mark them complete with a tap
- Work offline, sync resumes when WiFi returns
- Launch via a "magic notebook"

Phase 3 (handwriting-based task creation) awaits OCR library availability. But here's the beauty: the architecture supports it. When Tesseract lands on the device, we rebuild with `ENABLE_OCR` and it just works.

## Closing Thoughts

Building for embedded Linux with AI assistance is surprisingly effective. The combination of:

- **Structured methodology** (GSD phases/plans)
- **AI code generation** (Claude's implementation)
- **Human creativity** (launcher concept, UX decisions)
- **Real hardware testing** (no emulator can replace e-ink feel)

...produces software that ships.

The reMarkable is a fascinating platform—constrained, opinionated, with a clear design philosophy. Building native apps for it requires understanding Qt, embedded Linux, cross-compilation, and e-ink UX. But with the right tools (human + AI), it's absolutely doable.

If you're curious about AI-assisted development, I encourage you to try GSD. If you're curious about reMarkable development, grab a device and start hacking. And if you want your Todoist tasks on e-ink, well, the code is out there.

Now if you'll excuse me, I have tasks to complete. On my reMarkable. With software I built. With Claude.

*Happy hacking.*

---

## Technical Appendix

### Build Environment

- **Host:** arm64 Linux (Raspberry Pi 5, Debian)
- **Target:** ARM32 (reMarkable 2, custom Linux)
- **Toolchain:** gcc-arm-linux-gnueabihf 14.2.0
- **Qt Version:** 6.8.2 (host and device)
- **Device Firmware:** 3.24.0.149

### Key Files

```
remarkable-todoist/
├── build-remarkable.sh          # Cross-compilation with auto-setup
├── scripts/pull-sysroot.sh      # Device library extraction
├── src/
│   ├── controllers/             # AppController (QML bridge)
│   ├── models/                  # Task, TaskModel, SyncQueue
│   ├── network/                 # TodoistClient, SyncManager
│   └── ocr/                     # HandwritingRecognizer (optional)
├── qml/
│   ├── main.qml                 # Main UI layout
│   └── TaskDelegate.qml         # Task list item component
└── launcher/
    ├── todoist-launcher.sh      # Notebook watcher
    └── todoist-launcher.service # systemd unit
```

### Resources

- **GSD Methodology:** [github.com/anthropics/claude-code](https://github.com/anthropics/claude-code) (see `/gsd` commands)
- **reMarkable API:** Undocumented, reverse-engineered from xochitl
- **Qt on reMarkable:** [remarkablewiki.com](https://remarkablewiki.com/devel/qt_creator)
- **Todoist API:** [developer.todoist.com](https://developer.todoist.com/rest/v2/)

### Contact

Questions? Find me on GitHub or the reMarkable Discord.

---

*Written in Markdown, on a real keyboard, by a human, with substantial AI assistance from Claude Sonnet 4.5.*

*Build date: 2026-02-06*

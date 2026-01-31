# Technology Stack

**Project:** reMarkable 2 Todoist Client
**Researched:** 2026-01-29
**Overall Confidence:** MEDIUM-HIGH

## Recommended Stack

### Core Framework
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **C++** | C++17 | Primary language | Native performance on ARM, direct framebuffer access, low memory footprint essential for 1GB RAM device | HIGH |
| **Qt Framework** | **Qt 6 (Quick/QML)** | GUI framework | **UPDATE 2026-01-31:** reMarkable 3.x uses Qt6 with Quick/QML, NOT Qt5 Widgets. Must use QML for UI. | HIGH |
| **rmKit** | Latest via Toltec | Alternative UI framework | AVOID for this project - requires custom framebuffer handling, adds complexity. Use Qt since it's pre-installed | MEDIUM |

**Rationale for Qt over rmKit:** Qt comes pre-installed on reMarkable 2 and provides battle-tested networking, JSON parsing, and event handling. rmKit is a single-header framework good for simple apps, but Qt's mature HTTP client and JSON APIs make Todoist integration straightforward. The added "framework weight" is irrelevant since Qt is already on the device.

**IMPORTANT UPDATE (2026-01-31):** Original research indicated Qt 5.15 Widgets. During integration testing, discovered reMarkable 3.x firmware actually uses **Qt6 with Qt Quick/QML**. The UI must be built with QML, not C++ widgets. This was validated by the actual device/toolchain requirements.

### Display System
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **rm2fb (display package)** | Latest via Toltec | Framebuffer compatibility | REQUIRED on reMarkable 2 for apps to display anything. Install via `opkg install display` | HIGH |
| **Qt EPaper** | Built into Qt 5.15 | E-ink optimization | Qt has native e-ink display support for reMarkable, handles partial refresh automatically | MEDIUM |

**Critical:** On reMarkable 2, you MUST install the `display` package (formerly `rm2fb`) or your app will soft-brick the device by not being able to render to screen. rm2fb provides framebuffer shim allowing rM1 apps to work on rM2's different display architecture.

### Networking & HTTP
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **QNetworkAccessManager** | Qt 5.15 | HTTP client | Built into Qt, handles HTTPS with OpenSSL, excellent for REST APIs, async request/reply model perfect for sync operations | HIGH |
| **OpenSSL** | 1.1+ (system) | TLS/SSL | Required for HTTPS to Todoist API, likely pre-installed on device, Qt links against system OpenSSL | MEDIUM |

**Note:** Verify OpenSSL is available on target device. Qt on reMarkable should have SSL support compiled in, but call `QSslSocket::supportsSsl()` at startup to confirm.

### Data Storage
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **QSettings** | Qt 5.15 | Configuration storage | Simple INI-based config for API tokens, last sync time, preferences. Stores in `/home/root/.config/` | HIGH |
| **Qt JSON (QJsonDocument)** | Qt 5.15 | Data serialization | Parse Todoist API responses, serialize offline queue. Qt's JSON API is fast and integrated | HIGH |
| **Plain files** | N/A | Offline queue | Store pending operations as JSON files in `/home/root/.local/share/remarkable-todoist/queue/`. Simple, robust, survives crashes | MEDIUM |

**Rationale against SQLite:** For this app's simple data model (list of tasks + offline queue), JSON files are sufficient and reduce dependencies. SQLite would be overkill for <1000 tasks. If scaling beyond 5000 tasks, reconsider.

### Todoist API Integration
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **Todoist REST API** | v2 | Task sync | Current official API, REST architecture, JSON responses. Simpler than Sync API v9 for read-heavy workload | HIGH |
| **Bearer Token Auth** | N/A | Authentication | Todoist uses `Authorization: Bearer $token` header, straightforward to implement with QNetworkRequest::setRawHeader() | HIGH |

**API Endpoints:**
- GET `https://api.todoist.com/rest/v2/tasks` - fetch all tasks
- POST `https://api.todoist.com/rest/v2/tasks` - create task
- GET `https://api.todoist.com/rest/v2/projects` - fetch projects for metadata

**Do NOT use Todoist Sync API v9** - it's complex, designed for bidirectional full sync. REST API v2 is sufficient for this app's pull-oriented workflow.

### Handwriting Recognition
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **Oxide API** | Via Oxide launcher | Capture handwriting | Use Oxide's existing handwriting capture, get raw stroke data | LOW |
| **MyScript Cloud API** | Cloud-based | Text conversion | reMarkable uses MyScript but API key is proprietary. Requires user to bring own MyScript developer account (HMAC + app keys) | LOW |
| **Alternative: Manual entry** | N/A | Text input | Qt text input widget as fallback if handwriting proves complex | HIGH |

**CRITICAL CONSTRAINT:** Handwriting-to-text is non-trivial. MyScript API requires:
1. Developer account (not free for production)
2. HMAC authentication with application/HMAC keys
3. Internet connection for cloud API call
4. Stroke data in specific format

**Recommendation:** Start with manual text entry (Qt QLineEdit) for MVP. Add handwriting as v2 feature after validating MyScript API feasibility. Don't block launch on this.

### Distribution & Package Management
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **Toltec** | Latest | Package repository | Community-standard package manager, uses opkg, supports reMarkable 2 | HIGH |
| **Oxide** | v2.8.4+ | Launcher | Recommended launcher for Toltec apps, provides app registration API, multitasking | HIGH |
| **opkg package (.ipk)** | N/A | Distribution format | Standard package format for Toltec, created via toltecmk tool | MEDIUM |

**Installation flow:**
1. User installs Toltec on device
2. User installs Oxide launcher: `opkg install oxide`
3. User installs app: `opkg install remarkable-todoist`
4. App auto-registers with Oxide via `rot apps call registerApplication`

### Build System & Toolchain
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **CMake** | 3.16+ | Build system | Standard for Qt projects, cross-platform | HIGH |
| **Official reMarkable SDK** | v3.1.15 | Cross-compilation | **UPDATED:** Use official SDK from remarkable.engineering for Qt6 support | HIGH |
| **Toltec Docker Toolchain** | Latest | Alternative | For Qt5/Widgets apps. Does NOT have Qt6 - avoid for this project | MEDIUM |
| **qmake** | N/A | Alternative build system | AVOID - CMake is more flexible and better documented for cross-compilation | MEDIUM |

**Official SDK (RECOMMENDED for Qt6):**
```bash
# Download and install official toolchain
wget https://remarkable.engineering/oecore-x86_64-cortexa7hf-neon-rm11x-toolchain-3.1.15.sh
chmod +x oecore-x86_64-cortexa7hf-neon-rm11x-toolchain-3.1.15.sh
./oecore-x86_64-cortexa7hf-neon-rm11x-toolchain-3.1.15.sh -d ~/remarkable-toolchain

# Use toolchain
source ~/remarkable-toolchain/environment-setup-cortexa7hf-neon-remarkable-linux-gnueabi
cmake -B build-rm -DCMAKE_TOOLCHAIN_FILE=cmake/remarkable.cmake
cmake --build build-rm
```

**CMake toolchain file (cmake/remarkable.cmake):**
- Uses environment variables from SDK: `$CC`, `$CXX`, `$SDKTARGETSYSROOT`
- ARM flags: `-march=armv7-a -mfpu=neon -mfloat-abi=hard` for Cortex-A7

**For Toltec packaging:** Use `toltecmk` tool to create .ipk package from recipe.

### Development Environment
| Technology | Version | Purpose | Why | Confidence |
|------------|---------|---------|-----|------------|
| **Qt Creator** | 6.0+ | IDE | Optional, has decent remote debugging support for embedded targets | MEDIUM |
| **VS Code** | Latest | Alternative IDE | Good C++ support with clangd, easier Docker integration | MEDIUM |
| **SSH** | N/A | Remote access | reMarkable provides SSH access for deployment and testing | HIGH |

**Deployment workflow:**
1. Cross-compile binary with Docker toolchain
2. SCP binary to device: `scp app root@remarkable:/opt/bin/`
3. SSH to device and test
4. Package with toltecmk when stable

## Alternatives Considered

| Category | Recommended | Alternative | Why Not | Confidence |
|----------|-------------|-------------|---------|------------|
| **Language** | C++ with Qt | Rust + libremarkable | Rust has steep learning curve, libremarkable less mature than Qt, no clear advantage for HTTP/JSON work | MEDIUM |
| **Language** | C++ with Qt | Python + Carta | Python is slower, 1GB RAM is tight, no official Python Qt bindings on device | HIGH |
| **Language** | C++ with Qt | .NET + ReMarkable.NET | .NET runtime overhead on embedded device, less community adoption | MEDIUM |
| **UI Framework** | Qt 5.15 | rmKit | rmKit requires manual framebuffer management, no built-in HTTP client, reinvents networking wheel | HIGH |
| **UI Framework** | Qt 5.15 | Simple (SAS bash) | Good for trivial apps, but no HTTP library, would need curl + jq + bash parsing = fragile | HIGH |
| **API** | REST API v2 | Sync API v9 | Sync API is overkill for one-way-heavy sync, more complex delta handling, harder to debug | HIGH |
| **Display** | rm2fb + Qt | Custom framebuffer | Reinventing wheel, rm2fb is battle-tested, handles rM1/rM2 differences | HIGH |
| **Data storage** | JSON files | SQLite | SQLite adds dependency, JSON files sufficient for <1000 tasks, easier to debug | MEDIUM |
| **Data storage** | JSON files | Binary format | JSON is human-readable for debugging, compression not needed for task list size | HIGH |

## Dependency Installation

### On Device (via Toltec)
```bash
# Install Toltec (one-time, user does this)
wget http://bin.entware.net/armv7sf-k3.2/installer/generic.sh
sh generic.sh

# Install required packages
opkg update
opkg install display        # rm2fb framebuffer (REQUIRED for rM2)
opkg install oxide          # Launcher
```

### Development Machine
```bash
# Pull Toltec toolchain
docker pull ghcr.io/toltec-dev/toolchain:latest

# Install toltecmk for packaging
pip install toltecmk
```

### Build Dependencies (in Docker)
```cmake
# CMakeLists.txt (UPDATED 2026-01-31 for Qt6 Quick)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Qml Quick QuickControls2)
```

## Version Constraints & Compatibility

### Device Requirements
- **reMarkable 2** (ARMv7, 1.2GHz dual-core, 1GB RAM)
- **OS Version:** 2.6.1.71 - 3.3.2.1666 (Toltec supported range as of 2025)
- **CRITICAL:** OS 3.4+ has limited Toltec support. Check toltec-dev/toltec#859 for status.

### Qt Version Constraints
- **~~Qt 5.15~~** **Qt 6** is used on reMarkable 3.x firmware
- **DO NOT** install Qt via Toltec - conflicts with system Qt
- **Use system Qt** only: cross-compile against reMarkable's Qt 6
- **Qt Quick/QML ONLY** - Qt Widgets do NOT work. This was **VERIFIED 2026-01-31**.
  - Original contradiction resolved: Community may have been using older firmware. 3.x requires QML.

### Known Compatibility Issues
| Issue | Impact | Mitigation |
|-------|--------|------------|
| OS 3.4+ Toltec support incomplete | App may not install on latest OS | Warn users to check OS version, track toltec-dev/toltec#859 |
| rm2fb missing on rM2 | App won't display, soft-brick | Make `display` package a mandatory dependency in package recipe |
| OpenSSL version mismatch | HTTPS fails | Dynamically link OpenSSL, rely on system version |
| Qt Widgets vs Qt Quick | API differences | **RESOLVED:** Must use Qt Quick/QML. Widgets do NOT work on rM 3.x. |
| MyScript API rate limits | Handwriting conversion fails | Defer handwriting to v2, not MVP blocker |

## Performance Considerations

### Resource Constraints
- **RAM:** 1GB total, ~600MB available for apps after OS overhead
- **CPU:** 1.2GHz dual-core ARM, modest performance
- **Display:** E-ink refresh is slow (~250ms full refresh, ~100ms partial)
- **Network:** WiFi only, users disable it to save battery

### Optimization Strategies
| Concern | Strategy | Why |
|---------|----------|-----|
| **Memory** | Use Qt's implicit sharing (QString, QJsonObject) | Reduces copy overhead |
| **Memory** | Limit task cache to 1000 tasks | 1000 tasks * 1KB avg = ~1MB, acceptable |
| **Display** | Use Qt's EPaper platform plugin | Handles partial refresh automatically |
| **Display** | Update UI in chunks, not per-task | Batch UI updates to minimize redraws |
| **Battery** | Respect WiFi state | Only sync when WiFi enabled, queue operations otherwise |
| **Battery** | Cache API responses for 5 minutes | Avoid redundant requests when user switches views |
| **Network** | Timeout requests after 10s | Slow connections on device, fail fast |
| **Startup** | Lazy-load tasks on first view | Don't block app launch on network request |

### E-ink Display Best Practices
- **Minimize full refreshes:** Use partial refresh for list updates
- **Avoid animations:** E-ink can't handle 60fps, Qt will handle this
- **High contrast UI:** Black text on white background, no gradients
- **Large touch targets:** Minimum 44x44px for checkboxes/buttons
- **Static layout:** Avoid dynamic resize, helps Qt optimize refreshes

## Security Considerations

| Concern | Mitigation | Priority |
|---------|------------|----------|
| **API token storage** | Store in QSettings with restrictive file permissions (600) | HIGH |
| **HTTPS certificate validation** | QNetworkAccessManager validates by default, don't disable | HIGH |
| **Offline queue tampering** | Accept risk - device is single-user, physical access = compromise | LOW |
| **Credential leak in logs** | Never log API token, redact in debug output | HIGH |

**Note:** reMarkable has SSH enabled by default with simple password. Device security model assumes physical possession = full control. Don't over-engineer.

## Confidence Assessment

| Component | Confidence | Reason |
|-----------|------------|--------|
| Qt Framework | HIGH | Official support, pre-installed, extensive documentation |
| Qt Networking | HIGH | QNetworkAccessManager is mature, SSL support verified in docs |
| Qt JSON | HIGH | QJsonDocument is standard, used across Qt ecosystem |
| Toltec distribution | MEDIUM-HIGH | Community-maintained, active but dependent on OS version support |
| rm2fb (display) | HIGH | Required and widely used, battle-tested |
| Todoist REST API v2 | HIGH | Official API, JSON responses, well-documented |
| Handwriting recognition | LOW | MyScript API requires paid developer account, complex integration |
| CMake build system | HIGH | Standard for Qt cross-compilation |
| Toltec toolchain | MEDIUM-HIGH | Official Docker images, actively maintained |
| Battery/performance | MEDIUM | Based on community reports and device specs, not direct testing |

## Open Questions & Risks

### Needs Early Validation
1. **~~Qt Widgets vs Qt Quick:~~** **RESOLVED 2026-01-31:** reMarkable 3.x firmware uses **Qt6 with Qt Quick/QML**. Qt Widgets do NOT work - must use Qt Quick. Discovered during 01-04 integration. Refactored entire UI to QML.
2. **OpenSSL availability:** Assume system has OpenSSL but verify with `QSslSocket::supportsSsl()` at runtime.
3. **rm2fb installation:** Is it automatic dependency or user must install manually? Check toltecmk dependency handling.
4. **Task list size:** Performance at 1000+ tasks unknown. Load test with large JSON response.

### Deferred to Later Phases
1. **Handwriting recognition:** Complex, requires MyScript account. Defer to post-MVP.
2. **Background sync:** Would require daemon/service. Start with manual refresh.
3. **Conflict resolution:** Offline edits conflicting with server state. MVP can reject conflicts.
4. **Multi-account support:** Single account hardcoded for MVP simplicity.

## Sources

### Official Documentation
- [reMarkable Developer](https://developer.remarkable.com/documentation/software-stack)
- [reMarkable Guide - Programming Languages](https://remarkable.guide/devel/language/index.html)
- [reMarkable Guide - Qt Framework](https://remarkable.guide/devel/language/c++/qt.html)
- [reMarkable Guide - Toolchains](https://remarkable.guide/devel/toolchains.html)
- [reMarkable Guide - Display](https://remarkable.guide/devel/device/display.html)
- [Qt 5.15 Documentation - QJsonDocument](https://doc.qt.io/qt-5/qjsondocument.html)
- [Qt 6.10 Documentation - QNetworkAccessManager](https://doc.qt.io/qt-6/qnetworkaccessmanager.html)

### Community Resources
- [Toltec Package Repository](https://github.com/toltec-dev/toltec)
- [Toltec Official Site](https://toltec-dev.org/)
- [Oxide Launcher](https://github.com/Eeems-Org/oxide)
- [rmKit Framework](https://github.com/rmkit-dev/rmkit)
- [rm2fb Framebuffer](https://github.com/ddvk/remarkable2-framebuffer)
- [Writing a Simple Oxide Application](https://eeems.website/writing-a-simple-oxide-application/)
- [Awesome reMarkable](https://github.com/reHackable/awesome-reMarkable)

### API Documentation
- [Todoist REST API v2](https://developer.todoist.com/rest/v2/)
- [Todoist API Guides](https://developer.todoist.com/guides/)
- [MyScript SDK](https://www.myscript.com/sdk/)

### Technical References
- [reMarkable Filesystem Info](https://remarkable.jms1.info/info/filesystem.html)
- [reMarkable Paper Pro Specs](https://remarkable.com/products/remarkable-2)
- [Qt JSON Support](https://doc.qt.io/qt-6/json.html)
- [Qt Networking](https://doc.qt.io/qt-6/qtnetwork-programming.html)

**Research Confidence:** MEDIUM-HIGH
**Last Updated:** 2026-01-29
**Next Review:** After toolchain setup and Qt verification on device

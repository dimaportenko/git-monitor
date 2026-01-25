# Git Monitor (gm)

A terminal UI application for monitoring Git-related activities, starting with GitHub Actions workflow status tracking.

## Purpose

This project serves as a hands-on learning exercise for modern C++ development. The goal is to build a practical, useful tool while exploring C++23 features, async programming patterns, and TUI development.

## Tech Stack

| Component | Technology |
|-----------|------------|
| Language | C++23 |
| Build System | CMake |
| Package Manager | vcpkg |
| TUI Framework | ftxui |
| HTTP Client | cpp-httplib or libcurl |
| JSON Parser | nlohmann/json |
| Configuration | TOML (toml++) |
| Notifications | macOS Notification Center (via osascript or terminal-notifier) |

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Application                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   Config    │  │    State    │  │     Notification    │  │
│  │   Manager   │  │    Store    │  │   Service (macOS)   │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
│         │                │                    │             │
│         └────────────────┼────────────────────┘             │
│                          │                                  │
│  ┌───────────────────────┴───────────────────────────────┐  │
│  │                    Core Engine                        │  │
│  │  - Workflow polling loop                              │  │
│  │  - Status change detection                            │  │
│  │  - Event dispatching                                  │  │
│  └───────────────────────────────────────────────────────┘  │
│                          │                                  │
│  ┌───────────────────────┴───────────────────────────────┐  │
│  │                   GitHub Client                       │  │
│  │  - REST API wrapper                                   │  │
│  │  - Authentication (PAT)                               │  │
│  │  - Rate limit handling                                │  │
│  └───────────────────────────────────────────────────────┘  │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                      UI Layer (ftxui)                       │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐ │
│  │  Workflow    │ │   Status     │ │      Settings        │ │
│  │  List View   │ │   Details    │ │        View          │ │
│  └──────────────┘ └──────────────┘ └──────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Directory Structure

```
gm/
├── CMakeLists.txt
├── vcpkg.json
├── README.md
├── PROJECT.md
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── application.hpp
│   │   └── application.cpp
│   ├── config/
│   │   ├── config.hpp
│   │   └── config.cpp
│   ├── github/
│   │   ├── client.hpp
│   │   ├── client.cpp
│   │   ├── types.hpp
│   │   └── rate_limiter.hpp
│   ├── core/
│   │   ├── engine.hpp
│   │   ├── engine.cpp
│   │   ├── state.hpp
│   │   └── events.hpp
│   ├── ui/
│   │   ├── components/
│   │   │   ├── workflow_list.hpp
│   │   │   ├── status_badge.hpp
│   │   │   └── header.hpp
│   │   ├── screens/
│   │   │   ├── main_screen.hpp
│   │   │   └── settings_screen.hpp
│   │   └── theme.hpp
│   └── notify/
│       ├── notifier.hpp
│       └── notifier.cpp
├── include/
│   └── gm/
│       └── version.hpp
└── tests/
    ├── CMakeLists.txt
    └── ...
```

## Features

### MVP (v0.1)

- [ ] Watch GitHub Actions workflows for specified repositories
- [ ] Display workflow status in terminal UI (success, failure, running, pending)
- [ ] Detect status changes and trigger desktop notifications
- [ ] Configuration file for watched repositories and settings
- [ ] Manual refresh capability
- [ ] Configurable polling interval

### Future Versions

- [ ] Multiple repository support with grouped view
- [ ] Workflow run history
- [ ] Log viewer for failed runs
- [ ] Branch filtering
- [ ] Keyboard shortcuts customization
- [ ] Support for GitLab CI/CD
- [ ] Support for local git repository status

## Configuration

Configuration stored in `~/.config/gm/config.toml` (or `~/Library/Application Support/gm/config.toml`):

```toml
[general]
polling_interval_seconds = 60
notifications_enabled = true

[github]
token = "ghp_xxxxxxxxxxxx"  # or use GM_GITHUB_TOKEN env var

[[watch]]
owner = "username"
repo = "repository-name"
workflows = ["ci.yml", "deploy.yml"]  # optional, watches all if omitted

[[watch]]
owner = "org-name"
repo = "another-repo"
```

## GitHub API Endpoints

Primary endpoints used:

- `GET /repos/{owner}/{repo}/actions/workflows` — List workflows
- `GET /repos/{owner}/{repo}/actions/workflows/{workflow_id}/runs` — Get workflow runs
- `GET /repos/{owner}/{repo}/actions/runs/{run_id}` — Get specific run details

Rate limits: 5000 requests/hour for authenticated requests.

## UI Mockup

```
┌─ Git Workflow Monitor ──────────────────────────────────────┐
│ ↻ Last updated: 2 min ago              [R]efresh  [Q]uit   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  user/repo-name                                             │
│  ├─ ● CI Pipeline                    ✓ success    12m ago  │
│  ├─ ● Deploy Production              ⟳ running    2m ago   │
│  └─ ● Nightly Tests                  ✗ failure    8h ago   │
│                                                             │
│  org/another-repo                                           │
│  ├─ ● Build & Test                   ✓ success    1h ago   │
│  └─ ● Security Scan                  ◯ pending    just now │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ Status: Watching 2 repos, 5 workflows | Next poll: 58s     │
└─────────────────────────────────────────────────────────────┘
```

## Development Guidelines

1. **Incremental development** — Build and test each component before integration
2. **Modern C++ idioms** — Use RAII, smart pointers, std::optional, std::expected
3. **Error handling** — Prefer std::expected (C++23) over exceptions where practical
4. **Async patterns** — Use coroutines for non-blocking API calls
5. **Testing** — Unit tests for core logic, especially GitHub client parsing

## Platform

**Target OS:** macOS

### macOS-Specific Considerations

- **Notifications:** Use `osascript` for native Notification Center integration, or `terminal-notifier` (installable via Homebrew) for more features
- **Compiler:** Apple Clang (Xcode Command Line Tools) or Homebrew LLVM/GCC for full C++23 support
- **Config location:** `~/Library/Application Support/gm/` (macOS standard) or `~/.config/gm/` (XDG standard)

### Notification Implementation

```cpp
// Simple macOS notification via osascript
void notify(const std::string& title, const std::string& message) {
    std::string cmd = std::format(
        R"(osascript -e 'display notification "{}" with title "{}"')",
        message, title
    );
    std::system(cmd.c_str());
}
```

## Building

### Prerequisites

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install vcpkg (if not already installed)
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh

# Optional: Install terminal-notifier for enhanced notifications
brew install terminal-notifier
```

### Build Commands

```bash
# Configure with vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build

# Run
./build/git-monitor
```

## Dependencies (vcpkg.json)

```json
{
  "name": "git-monitor",
  "version": "0.1.0",
  "dependencies": [
    "ftxui",
    "nlohmann-json",
    "cpp-httplib",
    "tomlplusplus"
  ]
}
```

**Note:** vcpkg will automatically detect macOS and use the appropriate triplet (`x64-osx` or `arm64-osx` for Apple Silicon).

## Learning Topics Covered

- Modern CMake practices
- Package management with vcpkg
- C++23 features (std::expected, std::format, etc.)
- Asynchronous programming with coroutines
- Terminal UI development
- REST API consumption
- JSON parsing
- Configuration file handling
- Desktop notifications integration

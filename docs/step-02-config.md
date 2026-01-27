# Step 2: Configuration — Read Watched Repos from TOML

Load watched repositories and settings from a TOML config file, falling back to mock data when no config exists.

## 1. Add `tomlplusplus` to `vcpkg.json`

```json
{
  "name": "git-monitor",
  "version": "0.1.0",
  "dependencies": [
    "ftxui",
    "nlohmann-json",
    "tomlplusplus"
  ]
}
```

## 2. Create `src/config/config.hpp`

```cpp
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace gm {

struct WatchEntry {
  std::string owner;
  std::string repo;
  std::vector<std::string> workflows;  // empty = watch all
};

struct Config {
  int polling_interval_seconds = 60;
  bool notifications_enabled = true;
  std::string github_token;            // from file or env var
  std::vector<WatchEntry> watches;
};

// Load config from file, falls back to env vars for token
std::optional<Config> load_config();

// Returns ~/.config/gm/config.toml
std::filesystem::path config_path();

}  // namespace gm
```

## 3. Create `src/config/config.cpp`

```cpp
#include "config/config.hpp"
#include <toml++/toml.hpp>
#include <cstdlib>

namespace gm {

std::filesystem::path config_path() {
  const char* home = std::getenv("HOME");
  if (!home) return {};
  return std::filesystem::path(home) / ".config" / "gm" / "config.toml";
}

std::optional<Config> load_config() {
  Config cfg;

  // Try env var for token first
  if (const char* env_token = std::getenv("GM_GITHUB_TOKEN")) {
    cfg.github_token = env_token;
  }

  auto path = config_path();
  if (!std::filesystem::exists(path)) {
    return cfg.github_token.empty() ? std::nullopt : std::optional{cfg};
  }

  try {
    auto table = toml::parse_file(path.string());

    // [general]
    if (auto general = table["general"].as_table()) {
      cfg.polling_interval_seconds =
          general->get("polling_interval_seconds")->value_or(60);
      cfg.notifications_enabled =
          general->get("notifications_enabled")->value_or(true);
    }

    // [github]
    if (auto github = table["github"].as_table()) {
      if (cfg.github_token.empty()) {
        cfg.github_token = github->get("token")->value_or(std::string{});
      }
    }

    // [[watch]]
    if (auto watches = table["watch"].as_array()) {
      for (const auto& entry : *watches) {
        if (auto watch_table = entry.as_table()) {
          WatchEntry watch;
          watch.owner = watch_table->get("owner")->value_or(std::string{});
          watch.repo = watch_table->get("repo")->value_or(std::string{});
          if (auto workflow_arr = watch_table->get("workflows")->as_array()) {
            for (const auto& workflow_name : *workflow_arr) {
              if (auto val = workflow_name.value<std::string>()) {
                watch.workflows.push_back(*val);
              }
            }
          }
          cfg.watches.push_back(std::move(watch));
        }
      }
    }

    return cfg;
  } catch (const toml::parse_error&) {
    return std::nullopt;
  }
}

}  // namespace gm
```

## 4. Update `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(git-monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ftxui CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

add_executable(git-monitor
  src/main.cpp
  src/config/config.cpp
)

target_include_directories(git-monitor PRIVATE src)

target_link_libraries(git-monitor PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
  tomlplusplus::tomlplusplus
)
```

## 5. Update `src/main.cpp`

Add config loading at the top of `main()` and update the footer:

```cpp
// Add include at top:
#include "config/config.hpp"

// In main(), before creating mock data:
auto config = gm::load_config();
if (config) {
  // For now, just show that config loaded — still use mock data for UI
  // We'll fetch real data in a later step
}
auto repos = create_mock_data();

// Update the footer text to show config status:
hbox({
  text(config
    ? "Config: " + std::to_string(config->watches.size()) + " repos"
    : "No config — using mock data"),
  filler(),
  text("Next poll: 58s"),
}) | dim,
```

## 6. Create sample config file

Create `~/.config/gm/config.toml`:

```toml
[general]
polling_interval_seconds = 60
notifications_enabled = true

[github]
token = ""  # or set GM_GITHUB_TOKEN env var

[[watch]]
owner = "your-username"
repo = "your-repo"
```

## 7. Build and run

```bash
cmake --preset default && cmake --build build -j 8 && ./build/git-monitor
```

The footer should show config status. The UI still uses mock data — real API fetching comes in a later step.

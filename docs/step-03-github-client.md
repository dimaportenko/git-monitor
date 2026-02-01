# Step 3: GitHub Client — Fetch Workflow Runs from API

Implement the GitHub REST API client to fetch actual workflow run data, replacing mock data with real GitHub Actions status.

## 1. Add `cpp-httplib` to `vcpkg.json`

```json
{
  "name": "git-monitor",
  "version": "0.1.0",
  "dependencies": [
    "ftxui",
    "nlohmann-json",
    "tomlplusplus",
    "cpp-httplib"
  ]
}
```

## 2. Create `src/github/client.hpp`

```cpp
#pragma once

#include <expected>
#include <string>
#include <vector>
#include "github/types.hpp"

namespace gm {

struct GitHubError {
  int status_code;
  std::string message;
};

class GitHubClient {
 public:
  explicit GitHubClient(std::string token);

  // Fetch recent workflow runs for a repository
  std::expected<std::vector<WorkflowRun>, GitHubError>
  fetch_workflow_runs(const std::string& owner,
                      const std::string& repo,
                      int per_page = 10);

 private:
  std::string token_;
  static constexpr const char* kApiHost = "api.github.com";
};

}  // namespace gm
```

## 3. Create `src/github/client.cpp`

```cpp
#include "github/client.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <format>
#include <iomanip>
#include <sstream>

namespace gm {

GitHubClient::GitHubClient(std::string token) : token_(std::move(token)) {}

// Parse ISO8601 timestamp to system_clock::time_point
std::chrono::system_clock::time_point parse_iso8601(const std::string& timestamp) {
  std::tm tm = {};
  std::istringstream ss(timestamp);
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
  return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

// Map GitHub status/conclusion to our WorkflowStatus
WorkflowStatus map_status(const std::string& status,
                          const std::string& conclusion) {
  if (status == "queued" || status == "waiting" || status == "pending") {
    return WorkflowStatus::Pending;
  }
  if (status == "in_progress") {
    return WorkflowStatus::Running;
  }
  // status == "completed"
  if (conclusion == "success") return WorkflowStatus::Success;
  if (conclusion == "failure") return WorkflowStatus::Failure;
  if (conclusion == "cancelled") return WorkflowStatus::Cancelled;
  return WorkflowStatus::Failure;  // timed_out, action_required, etc.
}

std::expected<std::vector<WorkflowRun>, GitHubError>
GitHubClient::fetch_workflow_runs(const std::string& owner,
                                  const std::string& repo,
                                  int per_page) {
  httplib::SSLClient client(kApiHost);
  client.set_bearer_token_auth(token_);
  client.set_default_headers({
    {"Accept", "application/vnd.github+json"},
    {"X-GitHub-Api-Version", "2022-11-28"},
    {"User-Agent", "git-monitor/0.1"}
  });

  std::string path = std::format("/repos/{}/{}/actions/runs?per_page={}",
                                  owner, repo, per_page);

  auto res = client.Get(path);

  if (!res) {
    return std::unexpected(GitHubError{0, "Connection failed"});
  }

  if (res->status != 200) {
    return std::unexpected(GitHubError{res->status, res->body});
  }

  try {
    auto json = nlohmann::json::parse(res->body);
    std::vector<WorkflowRun> runs;

    for (const auto& run : json["workflow_runs"]) {
      WorkflowRun workflow_run;
      workflow_run.name = run["name"].get<std::string>();
      workflow_run.status = map_status(
          run["status"].get<std::string>(),
          run.value("conclusion", ""));
      workflow_run.updated_at = parse_iso8601(
          run["updated_at"].get<std::string>());
      runs.push_back(std::move(workflow_run));
    }

    return runs;
  } catch (const nlohmann::json::exception& e) {
    return std::unexpected(GitHubError{-1, std::string("JSON parse error: ") + e.what()});
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
find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(git-monitor
  src/main.cpp
  src/config/config.cpp
  src/github/client.cpp
)

target_include_directories(git-monitor PRIVATE src)

target_link_libraries(git-monitor PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
  tomlplusplus::tomlplusplus
  httplib::httplib
  nlohmann_json::nlohmann_json
  OpenSSL::SSL
  OpenSSL::Crypto
)
```

## 5. Update `src/main.cpp`

Replace mock data fetching with real API calls:

```cpp
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "config/config.hpp"
#include "github/client.hpp"
#include "github/types.hpp"

using namespace ftxui;

// ... keep status_color, render_workflow, render_repository functions ...

// Fetch real data from GitHub API
std::vector<gm::Repository> fetch_repositories(
    gm::GitHubClient& client,
    const std::vector<gm::WatchEntry>& watches) {
  std::vector<gm::Repository> repos;

  for (const auto& watch : watches) {
    gm::Repository repo;
    repo.owner = watch.owner;
    repo.repo = watch.repo;

    auto result = client.fetch_workflow_runs(watch.owner, watch.repo);
    if (result) {
      repo.runs = std::move(*result);
    }
    // If fetch fails, repo.runs stays empty

    repos.push_back(std::move(repo));
  }

  return repos;
}

// Keep mock data as fallback
std::vector<gm::Repository> create_mock_data() {
  // ... existing mock data function ...
}

int main() {
  auto screen = ScreenInteractive::Fullscreen();

  auto config = gm::load_config();
  std::vector<gm::Repository> repos;
  std::string status_text;

  if (config && !config->github_token.empty() && !config->watches.empty()) {
    gm::GitHubClient client(config->github_token);
    repos = fetch_repositories(client, config->watches);
    status_text = "Watching " + std::to_string(config->watches.size()) + " repos";
  } else {
    repos = create_mock_data();
    status_text = "No config — using mock data";
  }

  auto component = Renderer([&] {
    // ... existing render logic ...
  });

  // ... existing event handling ...

  screen.Loop(component);
  return 0;
}
```

## 6. Configure and test

1. Ensure you have a GitHub Personal Access Token with `repo` and `actions:read` scopes
2. Add it to your config file or set the environment variable:

```bash
export GM_GITHUB_TOKEN="ghp_your_token_here"
```

3. Add a repository to watch in `~/.config/gm/config.toml`:

```toml
[[watch]]
owner = "anthropics"
repo = "claude-code"
```

## 7. Build and run

```bash
# Reconfigure to pick up new dependencies
cmake --preset default

# Build
cmake --build build -j 8

# Run
./build/git-monitor
```

You should now see real workflow status from your configured repositories instead of mock data.

## Notes

- The client fetches the 10 most recent workflow runs per repository
- Rate limits: 5000 requests/hour with authentication
- Errors are silently ignored (repo shows with empty runs list) — proper error display comes later
- The UI doesn't refresh automatically yet — that comes in a later step

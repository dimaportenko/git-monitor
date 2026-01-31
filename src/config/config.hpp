#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace gm {

struct WatchEntry {
  std::string owner;
  std::string repo;
  std::vector<std::string> workflows;
};

struct Config {
  int polling_interval_seconds = 60;
  bool notifications_enabled = true;
  std::string github_token;
  std::vector<WatchEntry> watches;
};

std::optional<Config> load_config();

std::filesystem::path config_path();

} // namespace gm

#include "config.hpp"
#include <cstdlib>
#include <toml++/toml.hpp>

namespace gm {

constexpr char kGithubTokenEnv[] = "GM_GITHUB_TOKEN";

std::filesystem::path config_path() {
  const char *home = std::getenv("HOME");
  if (!home) {
    return {};
  }

  return std::filesystem::path(home) / ".config" / "gm" / "config.toml";
}

std::optional<Config> load_config() {
  Config cfg;

  if (const char *env_token = std::getenv(kGithubTokenEnv)) {
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
        cfg.github_token = github->get("token")->value_or("");
      }
    }

    // [[watch]]
    if (auto watches = table["watch"].as_array()) {
      for (const auto &entry : *watches) {
        if (auto watch_table = entry.as_table()) {
          WatchEntry watch;
          watch.owner = watch_table->get("owner")->value_or(std::string{});
          watch.repo = watch_table->get("repo")->value_or(std::string{});

          if (auto workflow_arr = watch_table->get("workflows")->as_array()) {
            for (const auto &workflow_name : *workflow_arr) {
              if (auto val = workflow_name.value<std::string>()) {
                watch.workflows.push_back(*val);
              }
            }
          }

          cfg.watches.push_back(watch);
        }
      }
    }

    return cfg;
  } catch (const std::exception &e) {
    return std::nullopt;
  }
}

} // namespace gm

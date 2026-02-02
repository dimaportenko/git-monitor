#include "github/client.hpp"
#include "github/types.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <format>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {
std::ofstream& debug_log() {
  static std::ofstream log_file("/tmp/gm-debug.log", std::ios::app);
  return log_file;
}
}  // namespace

namespace gm {

GitHubClient::GitHubClient(std::string token) : token_(std::move(token)) {}

std::chrono::system_clock::time_point parse_iso8601(
    const std::string& timestamp) {
  std::tm tm = {};
  std::istringstream ss(timestamp);
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
  return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

WorkflowStatus map_status(const std::string& status,
                          const std::string& conclusion) {
  if (status == "queued" || status == "waiting" || status == "pending") {
    return WorkflowStatus::Pending;
  }

  if (status == "in_progress") {
    return WorkflowStatus::Running;
  }

  // status == "completed"
  if (conclusion == "success") {
    return WorkflowStatus::Success;
  }
  if (conclusion == "failure") {
    return WorkflowStatus::Failure;
  }
  if (conclusion == "cancelled") {
    return WorkflowStatus::Cancelled;
  }

  return WorkflowStatus::Failure;
}

std::expected<std::vector<WorkflowRun>, GitHubError>
GitHubClient::fetch_workflow_runs(const std::string& owner,
                                  const std::string& repo,
                                  int per_page) {
  httplib::SSLClient client(kApiHost);
  client.set_connection_timeout(10);  // 10 seconds to connect
  client.set_read_timeout(30);        // 30 seconds to read response
  client.set_bearer_token_auth(token_);
  client.set_default_headers({ { "Accept", "application/vnd.github+json" },
                               { "X-GitHub-Api-Version", "2022-11-28" },
                               { "User-Agent", "git-monitor/0.1" } });

  std::string path = std::format("/repos/{}/{}/actions/runs?per_page={}", owner,
                                 repo, per_page);

  debug_log() << "[REQUEST] GET " << path << std::endl;
  std::cout << "[REQUEST] GET " << path << std::endl;

  auto res = client.Get(path);

  std::cout << "[RESPONSE] Status: " << res->status << std::endl;

  if (!res) {
    auto error_msg = "HTTP request failed: " + httplib::to_string(res.error());
    debug_log() << "[ERROR] " << error_msg << std::endl;
    return std::unexpected(GitHubError{ 0, error_msg });
  }

  debug_log() << "[RESPONSE] Status: " << res->status << std::endl;
  debug_log() << "[RESPONSE] Body: " << res->body.substr(0, 1000) << std::endl;

  if (res->status != 200) {
    return std::unexpected(GitHubError{ res->status, res->body });
  }

  try {
    auto json = nlohmann::json::parse(res->body);
    std::vector<WorkflowRun> runs;

    for (const auto& run : json["workflow_runs"]) {
      WorkflowRun workflow_run;
      workflow_run.name = run["name"].get<std::string>();
      workflow_run.status = map_status(run["status"].get<std::string>(),
                                       run.value("conclusion", ""));
      workflow_run.updated_at =
          parse_iso8601(run["updated_at"].get<std::string>());
      runs.push_back(std::move(workflow_run));
    }

    debug_log() << "[PARSED] " << runs.size() << " workflow runs" << std::endl;
    return runs;
  } catch (const nlohmann::json::exception& error) {
    debug_log() << "[JSON ERROR] " << error.what() << std::endl;
    return std::unexpected(GitHubError{ res->status, error.what() });
  }
}

}  // namespace gm

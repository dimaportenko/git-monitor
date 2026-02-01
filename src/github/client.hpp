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

  std::expected<std::vector<WorkflowRun>, GitHubError> fetch_workflow_runs(
      const std::string& owner,
      const std::string& repo,
      int per_page = 10);

 private:
  std::string token_;
  static constexpr const char* kApiHost = "api.github.com";
};

}  // namespace gm

#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace gm {

enum class WorkflowStatus { Success, Failure, Running, Pending, Cancelled };

struct WorkflowRun {
  std::string name;
  WorkflowStatus status;
  std::chrono::system_clock::time_point updated_at;
};

struct Repository {
  std::string owner;
  std::string repo;
  std::vector<WorkflowRun> runs;
};

inline std::string status_symbol(WorkflowStatus status) {
  switch (status) {
  case WorkflowStatus::Success:
    return "✓";
  case WorkflowStatus::Failure:
    return "✗";
  case WorkflowStatus::Running:
    return "⟳";
  case WorkflowStatus::Pending:
    return "◯";
  case WorkflowStatus::Cancelled:
    return "⊘";
  }
  return "?";
}

inline std::string status_text(WorkflowStatus status) {
  switch (status) {
  case WorkflowStatus::Success:
    return "success";
  case WorkflowStatus::Failure:
    return "failure";
  case WorkflowStatus::Running:
    return "running";
  case WorkflowStatus::Pending:
    return "pending";
  case WorkflowStatus::Cancelled:
    return "cancelled";
  }
  return "unknown";
}

inline std::string time_ago(std::chrono::system_clock::time_point tp) {
  auto now = std::chrono::system_clock::now();
  auto diff =
      std::chrono::duration_cast<std::chrono::minutes>(now - tp).count();

  if (diff < 1)
    return "just now";
  if (diff < 60)
    return std::to_string(diff) + "m ago";
  if (diff < 1440)
    return std::to_string(diff / 60) + "h ago";
  return std::to_string(diff / 1440) + "d ago";
}

} // namespace gm

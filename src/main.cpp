#include <chrono>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "github/types.hpp"

using namespace ftxui;

// Cteate mock data for testign UI
std::vector<gm::Repository> create_mock_data() {
  using namespace std::chrono;
  auto now = system_clock::now();

  return {
      {"user",
       "repo-name",
       {
           {"CI Pipeline", gm::WorkflowStatus::Success, now - minutes(12)},
           {"Deploy Production", gm::WorkflowStatus::Running, now - minutes(2)},
           {"Nightly Tests", gm::WorkflowStatus::Failure, now - hours(8)},
       }},
      {"org",
       "another-repo",
       {
           {"Build & Test", gm::WorkflowStatus::Success, now - hours(1)},
           {"Security Scan", gm::WorkflowStatus::Pending, now - seconds(30)},
       }}};
}

Color status_color(gm::WorkflowStatus status) {
  switch (status) {
  case gm::WorkflowStatus::Success:
    return Color::Green;
  case gm::WorkflowStatus::Failure:
    return Color::Red;
  case gm::WorkflowStatus::Running:
    return Color::Yellow;
  case gm::WorkflowStatus::Pending:
    return Color::GrayLight;
  case gm::WorkflowStatus::Cancelled:
    return Color::GrayDark;
  }
  return Color::White;
}

Element render_workflow(const gm::WorkflowRun &run, bool is_last) {
  std::string prefix = is_last ? "└─ ● " : "├─ ● ";

  auto status_elem = hbox(
      {text(gm::status_symbol(run.status)) | color(status_color(run.status)),
       text(" " + gm::status_text(run.status)) |
           color(status_color(run.status))});

  return hbox({
      text(prefix),
      text(run.name) | flex,
      status_elem | size(WIDTH, EQUAL, 14),
      text(gm::time_ago(run.updated_at)) | size(WIDTH, EQUAL, 10) | align_right,
  });
}

Element render_repository(const gm::Repository &repo) {
  Elements workflow_elements;

  for (size_t i = 0; i < repo.runs.size(); ++i) {
    bool is_last = (i == repo.runs.size() - 1);
    workflow_elements.push_back(render_workflow(repo.runs[i], is_last));
  }

  return vbox({text(repo.owner + "/" + repo.repo) | bold,
               vbox(workflow_elements), text("")});
}

int main() {
  auto screen = ScreenInteractive::Fullscreen();
  auto repos = create_mock_data();

  auto component = Renderer([&] {
    Elements repo_elements;
    for (const auto &repo : repos) {
      repo_elements.push_back(render_repository(repo));
    }

    return vbox({
               // Header
               hbox({
                   text("Git Monitor") | bold,
                   filler(),
                   text("↻ Last updated: just now"),
                   text("  "),
                   text("[R]efresh") | dim,
                   text("  "),
                   text("[Q]uit") | dim,
               }),
               separator(),
               // Content
               vbox(repo_elements) | flex,
               // Footer
               separator(),
               hbox({
                   text("Watching " + std::to_string(repos.size()) + " repos"),
                   filler(),
                   text("Next poll: 58s") | dim,
               }),

           }) |
           border;
  });

  component = CatchEvent(component, [&](Event event) {
    if (event == Event::Character('q') || event == Event::Character('Q')) {
      screen.Exit();
      return true;
    }

    if (event == Event::Character('r') || event == Event::Character('R')) {
      // TODO: refresh logic
      return true;
    }

    return false;
  });

  screen.Loop(component);

  return 0;
}

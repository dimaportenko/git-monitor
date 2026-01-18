#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

int main() {
  using namespace ftxui;

  auto screen = ScreenInteractive::Fullscreen();

  auto component = Renderer([] {
    return vbox({
               text("Git Monitor") | bold | center,
               separator(),
               text("Press 'q' to quit") | center,
           }) |
           border; // | center;
  });

  component = CatchEvent(component, [&](Event event) {
    if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }
    return false;
  });

  screen.Loop(component);
  return 0;
}

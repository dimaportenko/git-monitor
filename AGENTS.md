# AGENTS.md (repo guidance for coding agents)

This repository is a small C++23 terminal UI app built with CMake + vcpkg and
`ftxui`. This file is intended to help automated coding agents quickly build,
run, and make style-consistent changes.

## Project layout

- `CMakeLists.txt`: single executable target `git-monitor`.
- `CMakePresets.json`: preset named `default` (currently contains a user-local
  vcpkg toolchain path).
- `vcpkg.json`: vcpkg manifest (dependency: `ftxui`).
- `src/main.cpp`: current entrypoint.
- `build/`: out-of-source build directory (generated).

## Build / run commands (CMake)

### Configure (recommended)

Prefer configuring into `build/` and generating `compile_commands.json`.

- Configure with a preset:
  - `cmake --preset default`

Notes:
- The current `CMakePresets.json` hard-codes a toolchain file:
  `CMAKE_TOOLCHAIN_FILE=/Users/dmitriyportenko/vcpkg/scripts/buildsystems/vcpkg.cmake`.
  If that path doesn’t exist on your machine/CI:
  - Set `VCPKG_ROOT` and run:
    - `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"`

### Build

- `cmake --build build -j 8`

### Run

This is an interactive fullscreen TUI.

- `./build/git-monitor`

Tips:
- The UI exits on `q` (see `src/main.cpp`).
- In automation, avoid running the TUI unless you can provide a TTY.

### Clean rebuild

CMake doesn’t define a universal “clean” across generators; prefer deleting the
build directory.

- `rm -rf build && cmake -S . -B build && cmake --build build`

## Lint / format

No formatter/linter config is currently committed (no `.clang-format`, no
`.clang-tidy`). Use the conventions below and prefer adding config only when
requested.

### Formatting (manual, unless config added)

Recommended local tooling:
- `clang-format` (if installed)

If a `.clang-format` is later added, apply it consistently:
- Format one file:
  - `clang-format -i src/main.cpp`
- Format all sources (example):
  - `clang-format -i src/**/*.cpp src/**/*.hpp src/**/*.h`

### Static analysis (optional)

If `clang-tidy` is available, use the build’s `compile_commands.json`.

- Configure/build first to generate compilation database:
  - `cmake -S . -B build`
- Run clang-tidy on one file:
  - `clang-tidy -p build src/main.cpp --`

(There is no `.clang-tidy` file yet; run with a conservative default set of
checks if needed and keep changes minimal.)

## Tests

This repo currently has **no test target** and does not enable CTest.

If/when tests are added via CMake/CTest, standard commands should be:

- Run all tests:
  - `ctest --test-dir build --output-on-failure`
- Run a single test by regex (recommended):
  - `ctest --test-dir build -R "<regex>" --output-on-failure`
- Re-run failed tests:
  - `ctest --test-dir build --rerun-failed --output-on-failure`

Guidance for adding tests (only if requested):
- Enable CTest: `include(CTest)` + `enable_testing()` in `CMakeLists.txt`.
- Add a `tests/` directory and register tests with `add_test()`.
- Prefer a lightweight framework (e.g., Catch2 or GoogleTest via vcpkg).

## Code style guidelines (C++23)

These conventions should be applied to any new or edited code.

### Formatting

- Indentation: 2 spaces (no tabs).
- Braces: K&R style (opening brace on same line).
- Keep lines reasonably short (~100 columns) where practical.
- Prefer `const` and `constexpr` for immutability.

### Includes

- Order includes top-to-bottom:
  1) Standard library headers (`<vector>`, `<string>`, …)
  2) Third-party headers (`<ftxui/...>`)
  3) Project headers (`"..."`)
- Keep includes minimal (“include what you use”).
- Avoid `using namespace` in headers. In `.cpp`, it’s acceptable in a narrow
  scope (e.g., inside `main()`), as seen in `src/main.cpp`.

### Naming

- Types (classes/structs/enums): `PascalCase`.
- Functions and variables: `snake_case`.
- Constants: `kPascalCase` or `snake_case` with `constexpr` (pick one per file
  and stay consistent).
- Files: `snake_case.cpp` / `snake_case.hpp` when adding new ones.

### Types / interfaces

- Prefer standard library types:
  - `std::string_view` for non-owning string parameters.
  - `std::span<T>` for non-owning array views.
  - `std::optional<T>` when a value may be absent.
  - `enum class` over plain enums.
- Avoid raw owning pointers; prefer RAII (`std::unique_ptr`, `std::shared_ptr`)
  only when ownership semantics are required.

### Error handling

- For UI/event code, prefer explicit control flow and clear early returns.
- Use exceptions sparingly; do not throw across event/render callbacks.
- When returning errors from non-UI logic:
  - Prefer `std::optional`/`std::variant`/error enums, or a small `Result` type
    (only introduce new abstractions when requested).
- Always provide actionable error messages when failing due to filesystem/git
  state (this app is expected to interact with git repos).

### FTXUI patterns

- Keep render functions pure where possible (no hidden side effects).
- Avoid expensive work inside `Renderer` lambdas; precompute outside the render
  path or cache results.
- Keep event handlers (`CatchEvent`) small and focused.

### Performance / safety

- Avoid unnecessary allocations inside the render loop.
- Prefer `std::chrono` for time.
- Prefer range-based loops where readable.

## Repository-specific notes for agents

- Generated files:
  - `build/` is generated; don’t edit files under `build/`.
- Tooling output:
  - `CMAKE_EXPORT_COMPILE_COMMANDS` is enabled; `build/compile_commands.json`
    should exist after configuration.

## Cursor/Copilot rules

No Cursor rules found (`.cursor/rules/` or `.cursorrules`).
No Copilot instructions found (`.github/copilot-instructions.md`).

If these are added later, incorporate their requirements here and treat them as
higher priority than generic guidance.

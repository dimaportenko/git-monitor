# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Important: Educational Project

This is a learning project. **Provide code solutions but do not apply changes until explicitly asked.** The user will apply most code changes themselves for learning purposes.

## Project Overview

Git Monitor (gm) - a C++23 terminal UI application for monitoring GitHub Actions workflow status. Built with CMake, vcpkg, and FTXUI. Target platform is macOS.

## Build Commands

```bash
# Configure (uses vcpkg toolchain via preset)
cmake --preset default

# Alternative if preset doesn't work (set VCPKG_ROOT first)
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build build -j 8

# Run (interactive fullscreen TUI, exits with 'q')
./build/git-monitor

# Clean rebuild
rm -rf build && cmake --preset default && cmake --build build
```

## Architecture

The planned architecture (currently skeleton):
- **Core Engine**: Workflow polling, status change detection, event dispatching
- **GitHub Client**: REST API wrapper with PAT auth and rate limiting
- **UI Layer (FTXUI)**: Workflow list, status details, settings screens
- **Config Manager**: TOML config from `~/.config/gm/config.toml`
- **Notification Service**: macOS Notification Center via osascript

Planned directory structure under `src/`: `app/`, `config/`, `github/`, `core/`, `ui/`, `notify/`

## Dependencies (vcpkg)

Current: `ftxui`, `nlohmann-json`
Planned: `cpp-httplib`, `tomlplusplus`

## Code Style

- **Indentation**: 2 spaces, K&R braces
- **Naming**: PascalCase types, snake_case functions/variables
- **Includes order**: stdlib → third-party → project headers
- **Types**: Prefer `std::string_view`, `std::span`, `std::optional`, `std::expected`
- **FTXUI**: Keep render functions pure, avoid work inside `Renderer` lambdas

## Notes

- No tests currently; when added, use `ctest --test-dir build`
- No `.clang-format` or `.clang-tidy` yet
- `build/compile_commands.json` generated after configure

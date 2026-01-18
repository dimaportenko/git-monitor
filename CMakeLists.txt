cmake_minimum_required(VERSION 3.20)
project(git-monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ftxui CONFIG REQUIRED)

add_executable(git-monitor src/main.cpp)

target_link_libraries(git-monitor
    PRIVATE
        ftxui::screen
        ftxui::dom
        ftxui::component
)

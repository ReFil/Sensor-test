cmake_minimum_required(VERSION 3.13.1)
set(CONFIG_APPLICATION_DEFINED_SYSCALL true)

#set(ZEPHYR_EXTRA_MODULES "${ZMK_EXTRA_MODULES};${CMAKE_CURRENT_SOURCE_DIR}/module;${CMAKE_CURRENT_SOURCE_DIR}/keymap-module")

# Find Zephyr. This also loads Zephyr's build system.
find_package(Zephyr REQUIRED HINTS ../zephyr)
project(app)

target_include_directories(app PRIVATE include)
target_sources(app PRIVATE src/main.c)
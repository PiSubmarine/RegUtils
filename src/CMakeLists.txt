include(${_build_cmake_local})

add_library(PiSubmarine.RegUtils INTERFACE)
target_include_directories(PiSubmarine.RegUtils INTERFACE "../public")
PiSubmarineInitTarget(PiSubmarine.RegUtils)

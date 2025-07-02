
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(PiSubmarine.RegUtils INTERFACE)
target_include_directories(PiSubmarine.RegUtils INTERFACE "../public")

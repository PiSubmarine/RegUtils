cmake_minimum_required(VERSION 3.20)

include(FetchContent)

project(PiSubmarine.RegUtils.Test LANGUAGES CXX)

set(PiSubmarine.RegUtils.Test.Sources 
	"PiSubmarine/RegUtilsTest.cpp")

enable_testing()

# spdlog
find_package(spdlog)
find_package(gtest)

add_executable(PiSubmarine.RegUtils.Test ${PiSubmarine.RegUtils.Test.Sources})
target_compile_features(PiSubmarine.RegUtils.Test PRIVATE cxx_std_23)
target_link_libraries(PiSubmarine.RegUtils.Test PRIVATE PiSubmarine.RegUtils)
target_link_libraries(PiSubmarine.RegUtils.Test PRIVATE spdlog::spdlog)
target_link_libraries(PiSubmarine.RegUtils.Test PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(PiSubmarine.RegUtils.Test)
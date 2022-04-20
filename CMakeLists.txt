cmake_minimum_required(VERSION 3.9.0)

include(FetchContent)

project(safran CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

FetchContent_Declare(sqlite3 URL "http://sqlite.org/2021/sqlite-amalgamation-3340100.zip")
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(sqlite3)

message(STATUS "${sqlite3_SOURCE_DIR}")

include_directories(
	${PROJECT_SOURCE_DIR}/boost_1_76_0
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
  ${sqlite3_SOURCE_DIR}
)

file(GLOB all_src
 "${PROJECT_SOURCE_DIR}/include/*.h"
 "${PROJECT_SOURCE_DIR}/include/*.hpp"
 "${PROJECT_SOURCE_DIR}/src/*.cpp"
 "${sqlite3_SOURCE_DIR}/*.c"
 "${sqlite3_SOURCE_DIR}/*.h"
)

add_library(lsafran  ${all_src})
find_package(OpenMP REQUIRED)
target_link_libraries(lsafran PUBLIC OpenMP::OpenMP_CXX ${CMAKE_DL_LIBS})

add_executable(safran ${PROJECT_SOURCE_DIR}/Main.cpp)
target_link_libraries(safran PUBLIC lsafran)

install(TARGETS safran)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview3
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests tests/test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain lsafran)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)


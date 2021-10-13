cmake_minimum_required(VERSION 3.9.0)

include(FetchContent)

project(SAFRAN CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

FetchContent_Declare(sqlite3 URL "http://sqlite.org/2021/sqlite-amalgamation-3340100.zip")
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(sqlite3)

include_directories(
	${PROJECT_SOURCE_DIR}/boost_1_76_0
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
)

file(GLOB all_src
 "${PROJECT_SOURCE_DIR}/include/*.h"
 "${PROJECT_SOURCE_DIR}/include/*.hpp"
 "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

message(${sqlite3_SOURCE_DIR})
add_executable(SAFRAN  ${all_src})
target_include_directories(SAFRAN PUBLIC ${sqlite3_SOURCE_DIR})
target_sources(SAFRAN PUBLIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_link_libraries("${PROJECT_NAME}" PUBLIC OpenMP::OpenMP_CXX ${CMAKE_DL_LIBS})


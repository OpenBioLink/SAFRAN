cmake_minimum_required(VERSION 3.9.0)
project(safran CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
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

# set this to the path to OpenSSL if cmake cant find your OpenSSL installation
# set(OPENSSL_ROOT_DIR, "C:/Program Files/OpenSSL-Win64")

add_library(lsafran  ${all_src})
find_package(OpenMP REQUIRED)
target_link_libraries(lsafran PUBLIC OpenMP::OpenMP_CXX)

find_package(OpenSSL REQUIRED)
target_link_libraries(lsafran PRIVATE OpenSSL::applink)
target_link_libraries(lsafran PUBLIC OpenSSL::SSL)

add_executable(safran ${PROJECT_SOURCE_DIR}/Main.cpp)
target_link_libraries(safran lsafran)

install(TARGETS safran)

Include(FetchContent)

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



cmake_minimum_required(VERSION 3.9.0)
project(SAFRAN CXX)

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

add_executable(SAFRAN  ${all_src})
find_package(OpenMP REQUIRED)
target_link_libraries("${PROJECT_NAME}" PUBLIC OpenMP::OpenMP_CXX)


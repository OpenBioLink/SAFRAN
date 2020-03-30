cmake_minimum_required(VERSION 3.17.0)
project("IRIFAB" CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Release)

add_library("IRIFAB_LIB"
 "CSR.hpp"
 "FastRuleEngine.hpp"
 "Graph.hpp"
 "Index.hpp"
 "OriginalRuleEngine.hpp"
 "Properties.hpp"
 "Result.hpp"
 "Rule.hpp"
 "RuleEngine.hpp"
 "RuleReader.hpp"
 "ScoreTree.hpp"
 "TesttripleReader.hpp"
 "Types.h"
 "Util.hpp"
 "ValidationtripleReader.hpp"
)
set_target_properties("IRIFAB_LIB" PROPERTIES LINKER_LANGUAGE CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("IRIFAB_LIB" Threads::Threads)

add_executable("${PROJECT_NAME}" "Main.cpp")
target_link_libraries("${PROJECT_NAME}" "IRIFAB_LIB")

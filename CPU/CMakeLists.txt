cmake_minimum_required(VERSION 3.9.0)
project("IRIFAB" CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
add_executable("${PROJECT_NAME}"
 "Main.cpp"
 "CSR.hpp"
 "Graph.hpp"
 "TraintripleReader.hpp"
 "Clustering.hpp"
 "ClusteringEngine.hpp"
 "NoisyOrEngine.hpp"
 "Index.hpp"
 "Properties.hpp"
 "Rule.hpp"
 "RuleEngine.hpp"
 "RuleReader.hpp"
 "ScoreTree.hpp"
 "TesttripleReader.hpp"
 "Trial.hpp"
 "Types.h"
 "Util.hpp"
 "ValidationtripleReader.hpp"
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("IRIFAB" Threads::Threads)

find_package(OpenMP REQUIRED)
target_link_libraries("${PROJECT_NAME}" PUBLIC OpenMP::OpenMP_CXX)


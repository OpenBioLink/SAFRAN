cmake_minimum_required(VERSION 3.17.0)

project("IRIFAB")

set(CMAKE_BUILD_TYPE, Release)

add_executable("${PROJECT_NAME}"
 "Main.cpp"
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
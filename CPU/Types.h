#ifndef TYPES_H
#define TYPES_H

#include <unordered_map>
#include <unordered_set>
#include <set>
#include "Rule.hpp"

typedef std::set<int> Nodes;
typedef std::unordered_map<int, Nodes> NodeToNodes;
typedef std::unordered_map<int, NodeToNodes> RelNodeToNodes;

typedef std::unordered_map<int, std::unordered_set<Rule*>> RelToRules;

#endif //TYPES_H
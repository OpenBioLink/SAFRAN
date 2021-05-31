#ifndef RULEGRAPH_H
#define RULEGRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

#include "Types.h"
#include "Rule.h"
#include "TraintripleReader.h"
#include "TesttripleReader.h"
#include "ValidationtripleReader.h"
#include "Properties.hpp"

class RuleGraph {
public:
	RuleGraph(int nodesize, TraintripleReader* graph);
	RuleGraph(int nodesize, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr);
	void searchDFSSingleStart_filt(bool headNotTail, int filt_v, int v, Rule& r, bool bwd, std::vector<int>& solution, bool filtValidNotTest, bool filtExceptions);
	void searchDFSMultiStart_filt(bool headNotTail, int filt_v, Rule& r, bool bwd, std::vector<int>& solution, bool filtValidNotTest, bool filtExceptions);
	bool existsAcyclic(int* valId, Rule& rule, bool filtValidNotTest);
	void searchDFSSingleStart(int v, Rule& r, bool bwd, std::vector<int>& solution, int* previous, bool** visited);
	void searchDFSMultiStart(Rule& r, bool bwd, std::vector<int>& solution);

private:
	int size;
	TraintripleReader* graph;
	int* adj_lists;
	int* adj_list_starts;
	RelNodeToNodes train_relHeadToTails;
	RelNodeToNodes test_relHeadToTails;
	RelNodeToNodes valid_relHeadToTails;
	RelNodeToNodes train_relTailToHeads;
	RelNodeToNodes test_relTailToHeads;
	RelNodeToNodes valid_relTailToHeads;
	std::unordered_map<int, std::unordered_set<int>>* relCounter;

	void searchDFSUtil_filt(Rule* r, bool headNotTail, int filt_value, int filt_ex, int value, std::vector<int>& solution, int* relations, std::vector<std::vector<bool>>& visited, std::vector<int>& previous, int level, int rulelength, int limit, int*c, bool filtValidNotTest, bool filtExceptions, std::set<int>* trains, std::set<int>* second_filt_set);
	void searchDFSUtil(int value, std::vector<int>& solution, int* relations, bool** visited, int* previous, int level, int rulelength, int limit);
	bool existsAcyclic(int* valId, int* constant, int* relations, int N);

};

#endif //RULEGRAPH_H
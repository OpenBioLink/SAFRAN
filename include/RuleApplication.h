#ifndef CLUSTERINGAPPL_H
#define CLUSTERINGAPPL_H

#include <map>
#include <functional>

#include "Index.h"
#include "TraintripleReader.h"
#include "TesttripleReader.h"
#include "ValidationtripleReader.h"
#include "RuleReader.h"
#include "RuleGraph.h"
#include "Properties.hpp"
#include "MinHeap.h"
#include "Util.hpp"
#include "ScoreTree.h"

class RuleApplication
{
public:
	RuleApplication(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr);
	void apply_nr_noisy(std::unordered_map<int, std::pair<bool, std::vector<std::vector<int>>>> rel2clusters);
	void apply_only_noisy();
	void apply_only_max();

private:
	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;
	RuleReader* rr;

	FILE* pFile;
	RuleGraph* rulegraph;
	int reflexiv_token;

	void noisy(int rel, std::vector<std::vector<int>> clusters);
	void max(int rel, std::vector<std::vector<int>> clusters);
	void writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, double>> headresults, std::vector<std::pair<int, double>> tailresults, FILE* pFile, int& K);

	struct {
		bool operator()(std::pair<int, double> const& a, std::pair<int, double> const& b) const
		{
			return a.second > b.second;
		}
	} finalResultComperator;
};
#endif

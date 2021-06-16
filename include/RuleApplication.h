#ifndef CLUSTERINGAPPL_H
#define CLUSTERINGAPPL_H

#include <map>
#include <functional>
#include <math.h>
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
#include "boost/multiprecision/cpp_bin_float.hpp"

#include <iostream>
#include<iomanip>
#include<limits>

typedef boost::multiprecision::cpp_bin_float_50 float50;

class RuleApplication
{
public:
	RuleApplication(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr);
	void apply_nr_noisy(std::unordered_map<int, std::pair<std::pair<bool, std::vector<std::vector<int>>>, std::pair<bool, std::vector<std::vector<int>>>>> rel2clusters);
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
	int k;

	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> noisy(int rel, std::vector<std::vector<int>> clusters, bool predictHeadNotTail);
	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> max(int rel, std::vector<std::vector<int>> clusters, bool predictHeadNotTail);
	void writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, double>> headresults, std::vector<std::pair<int, double>> tailresults, FILE* pFile, int& K);
	void writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, float50>> headresults, std::vector<std::pair<int, float50>> tailresults, FILE* pFile, int& K);

	struct {
		bool operator()(std::pair<int, double> const& a, std::pair<int, double> const& b) const
		{
			return a.second > b.second;
		}
	} finalResultComperator;
};
#endif

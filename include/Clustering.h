#ifndef CL_H
#define CL_H

#include <algorithm>
#include <random>
#include <unordered_set>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <fstream>
#include <random>

#include "Index.h"
#include "TraintripleReader.h"
#include "TesttripleReader.h"
#include "ValidationtripleReader.h"
#include "RuleReader.h"
#include "Rule.h"
#include "Graph.h"
#include "RuleGraph.h"
#include "Properties.hpp"
#include "Util.hpp"
#include "ApplicationEngine.h"

class Clustering {
public:
	Clustering(int relation, int size, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr);
	std::string learn_cluster(std::string jacc_path);
private:
	int portions;
	uint32_t samples_size;
	uint32_t features_size;
	int relation;

	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;

	std::string strat;

	RuleReader* rr;
	int* adj_begin;
	Rule* rules_adj_list;
	int ind_ptr;
	int lenRules;

	int WORKER_THREADS;

	double max_c_c = 0.0;
	double max_ac1_ac1 = 0.0;
	double max_ac2_ac2 = 0.0;
	double max_c_ac2 = 0.0;
	double max_c_ac1 = 0.0;
	double max_ac1_ac2 = 0.0;
	double max_mrr = 0.0;
	std::vector<std::vector<int>> max_cluster;

	std::vector<double>* random_sample;

	static bool sortbysec(const std::pair<int, double>& a, const std::pair<int, double>& b)
	{
		return (a.second >= b.second);
	}

	std::vector<std::pair<int, double>>* read_jaccard(std::string path);
	void learn_parameters(Graph* g, RuleGraph* rulegraph);
};

#endif // CL_H
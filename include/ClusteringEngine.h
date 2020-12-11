#ifndef CLUSTERENGINE_H
#define CLUSTERENGINE_H

#include <iostream>
#include <omp.h>
#include <functional>

#include "Index.h"
#include "TraintripleReader.h"
#include "TesttripleReader.h"
#include "ValidationtripleReader.h"
#include "RuleReader.h"
#include "Rule.h"
#include "Properties.hpp"
#include "Clustering.h"
#include "Util.hpp"

class ClusteringEngine {

public:

	ClusteringEngine(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr);


	void learn();

	typedef std::function<bool(std::pair<int, double>, std::pair<int, double>)> Comparator;
	Comparator compFunctor =
		[](std::pair<int, double> elem1, std::pair<int, double> elem2)
	{
		return elem1.second > elem2.second;
	};


private:
	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;
	RuleReader* rr;
	int WORKER_THREADS;

	int* adj_lists;
	int* adj_list_starts;

	int* vt_adj_lists;
	int* vt_adj_list_starts;

	Rule* rules_adj_list;
	int* adj_begin;

	FILE* pFile;

	void writeThresh(std::string result);
};

#endif // CLUSTERENGINE_H

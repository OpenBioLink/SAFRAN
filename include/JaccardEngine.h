#ifndef JACCARDCALCULATOR_H
#define JACCARDCALCULATOR_H

#include <iostream>
#include <omp.h>

#include "Index.h"
#include "TraintripleReader.h"
#include "ValidationtripleReader.h"
#include "RuleReader.h"
#include "Rule.h"
#include "RuleGraph.h"
#include "MinHash.h"
#include "Properties.hpp"
#include "Util.hpp"

class JaccardEngine {

public:
	JaccardEngine(Index* index, TraintripleReader* graph, ValidationtripleReader* vtr, RuleReader* rr);
	void calculate_jaccard();

private:
	Index* index;
	TraintripleReader* graph;
	ValidationtripleReader* vtr;
	RuleReader* rr;
	int WORKER_THREADS;

	FILE* pFile;

	MinHash* min;
	int k;

	int* adj_lists;
	int* adj_list_starts;

	int* vt_adj_lists;
	int* vt_adj_list_starts;

	Rule* rules_adj_list;
	int* adj_begin;

	RuleGraph* rulegraph;

	void calc_sols(std::vector<long long>* solutions, Rule** rules, int ind_ptr, int len);
	void calc_jaccs(std::vector<long long>* solutions, Rule** rules, int len, std::vector<std::pair<int, double>>* jacc);


};

#endif // JACCARDCALCULATOR_H

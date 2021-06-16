#ifndef NOISYORENGINE_H
#define NOISYORENGINE_H

#include <map>
#include <functional>
#include "RuleGraph.h"
#include "Index.h"
#include "TraintripleReader.h"
#include "ValidationtripleReader.h"
#include "RuleReader.h"
#include "TesttripleReader.h"
#include "Util.hpp"
#include "Properties.hpp"
#include "MinHeap.h"
#include "ScoreTree.h"

#include "boost/multiprecision/cpp_bin_float.hpp"
typedef boost::multiprecision::cpp_bin_float_50 float50;

class ApplicationEngine
{
public:

	ApplicationEngine(int relation, RuleGraph* rulegraph, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr);

	std::pair<double,double> noisy(std::vector<std::vector<int>> clusters);
	std::pair<double,double> max(std::vector<std::vector<int>> clusters);

private:
	int relation;

	int k;

	int reflexiv_token;

	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;
	RuleReader* rr;
	RuleGraph* rulegraph;

	typedef std::function<bool(std::pair<int, double>, std::pair<int, double>)> Comparator;
	Comparator compFunctor =
		[](std::pair<int, double> elem1, std::pair<int, double> elem2)
	{
		return elem1.second > elem2.second;
	};

	struct {
		bool operator()(std::pair<int, double> const& a, std::pair<int, double> const& b) const
		{
			return a.second > b.second;
		}
	} finalResultComperator;
};


#endif //NoisyOR

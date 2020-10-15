#ifndef NOISYORENGINE_H
#define NOISYORENGINE_H

#include "RuleEngine.hpp"
#include "Util.hpp"
#include <map>
#include <functional>
#include "MinHeap.hpp"
#include "RuleGraph.hpp"
#define MAX_NODESIZE 200000

class NoisyOrEngine
{
public:

	NoisyOrEngine(int relation, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr, std::vector<std::vector<int>> clusters) {
		this->clusters = clusters;
		this->relation = relation;
		this->index = index;
		this->graph = graph;
		this->ttr = ttr;
		this->rr = rr;
		this->vtr = vtr;
		this->rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
	}

	~NoisyOrEngine() {
		delete rulegraph;
	}

	std::tuple<double, double, double> noisy() {

		int* adj_lists = graph->getCSR()->getAdjList();
		int* adj_list_starts = graph->getCSR()->getAdjBegin();

		Rule* rules_adj_list = rr->getCSR()->getAdjList();
		int* adj_begin = rr->getCSR()->getAdjBegin();

		int** testtriples = ttr->getTesttriples();
		int* testtriplessize = ttr->getTesttriplesSize();

		int* tt_adj_list = ttr->getCSR()->getAdjList();
		int* tt_adj_begin = ttr->getCSR()->getAdjBegin();

		int* vt_adj_list = vtr->getCSR()->getAdjList();
		int* vt_adj_begin = vtr->getCSR()->getAdjBegin();


		int rel = this->relation;
		int nodesize = index->getNodeSize();
		int ind_ptr = adj_begin[3 + rel];
		int lenRules = adj_begin[3 + rel + 1] - ind_ptr;

		double* result_head = new double[nodesize];
		double* result_tail = new double[nodesize];
		double* cluster_result_head = new double[nodesize];
		double* cluster_result_tail = new double[nodesize];
		std::fill(result_head, result_head + nodesize, 0.0);
		std::fill(result_tail, result_tail + nodesize, 0.0);
		std::fill(cluster_result_head, cluster_result_head + nodesize, 0.0);
		std::fill(cluster_result_tail, cluster_result_tail + nodesize, 0.0);

		

		{
			// adj list of testtriple x r ?
			int* v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2]]);
			int lenHeads = v_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
			auto heads = vtr->getRelHeadToTails()[rel].begin();
			while (heads != vtr->getRelHeadToTails()[rel].end()) {
				int head = heads->first;
				int* head_ind_ptr = &v_adj_list[3 + head];
				int lenTails = heads->second.size();


				if (lenTails > 0) {
					std::vector<int> touched_tails;
					for (int i = 0; i < clusters.size(); i++) {

						std::vector<int> touched_cluster_tails;

						for (auto ruleIndex : clusters[i]) {
							Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];

							std::vector<int> tailresults_vec;

							if (currRule.is_c()) {
								rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, false);
							}
							else {

								if (currRule.isBuffered()) {
									if (currRule.getRuletype() == Ruletype::XRule) {
										if (in_sorted(currRule.getBuffer(), head)) {
											tailresults_vec.push_back(*currRule.getHeadconstant());
										}
									}
									else if(currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()){
										tailresults_vec = currRule.getBuffer();
									}
								}
								else {
									if (currRule.getRuletype() == Ruletype::XRule) {
										std::vector<int> comp;
										rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, comp, false);
#pragma omp critical
										{
											if (!currRule.isBuffered())currRule.setBuffer(comp);
										}
										if (in_sorted(comp, head)) {
											tailresults_vec.push_back(*currRule.getHeadconstant());
										}
									}
									else if (currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
										rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, tailresults_vec, false);
#pragma omp critical
										{
											if (!currRule.isBuffered())currRule.setBuffer(tailresults_vec);
										}
									}
								}
							}

							if (tailresults_vec.size() > 0) {
								for (auto tailresult : tailresults_vec) {
									if (cluster_result_tail[tailresult] == 0.0) {
										cluster_result_tail[tailresult] = currRule.getAppliedConfidence();
										touched_cluster_tails.push_back(tailresult);
									}
									else {
										if (cluster_result_tail[tailresult] < currRule.getAppliedConfidence()) {
											cluster_result_tail[tailresult] = currRule.getAppliedConfidence();
										}
									}
								}
							}
						}
						for (auto i : touched_cluster_tails) {
							if (result_tail[i] == 0.0) {
								touched_tails.push_back(i);
							}
							result_tail[i] = 1.0 - (1.0 - result_tail[i]) * (1.0 - cluster_result_tail[i]);
							cluster_result_tail[i] = 0.0;
						}
					}

					for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
						int tail = v_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];

						MinHeap tails(10);
						for (auto i : touched_tails) {
							if (result_tail[i] >= tails.getMin().second) {
								if (i == tail || heads->second.find(i) == heads->second.end()) {
									tails.deleteMin();
									tails.insertKey(std::make_pair(i, result_tail[i]));
								}
							}
							result_tail[i] = 0.0;
						}

						std::vector<std::pair<int, double>> tailresults_vec;
						for (int i = 9; i >= 0; i--) {
							std::pair<int, double> tail_pred = tails.extractMin();
							if (tail_pred.first != -1) tailresults_vec.push_back(tail_pred);
						}
						std::reverse(tailresults_vec.begin(), tailresults_vec.end());

						int size_tails = tailresults_vec.size() > 10 ? 10 : tailresults_vec.size();
						int rank;
						for (int i = size_tails - 1; i >= 0; i--) {
							if (i == size_tails - 1 or tailresults_vec[i].second != tailresults_vec[i + 1].second) {
								rank = i;
							}
							if (tail == tailresults_vec[i].first) {
								if (rank == 0) {
									hit1++;
								}
								if (rank < 3) {
									hit3++;
								}
								hit10++;
							}
						}
						predicted++;
					}
				}
				heads++;
			}
		}
		{
			// adj list of testtriple x r ?
			int* v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2 + 1]]);
			int lenTails = v_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

			auto tails = vtr->getRelTailToHeads()[rel].begin();
			while (tails != vtr->getRelTailToHeads()[rel].end()) {
				int tail = tails->first;
				int* tail_ind_ptr = &v_adj_list[3 + tail];
				int lenHeads = tails->second.size();


				if (lenHeads > 0) {
					std::vector<int> touched_heads;
					for (int i = 0; i < clusters.size(); i++) {

						std::vector<int> touched_cluster_heads;

						for (auto ruleIndex : clusters[i]) {
							Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];

							std::vector<int> headresults_vec;

							if (currRule.is_c()) {
								rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, false);
							}
							else {
								if (currRule.isBuffered()) {
									if (currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
										headresults_vec = currRule.getBuffer();
									}
									else if(currRule.getRuletype() == Ruletype::YRule){
										if (in_sorted(currRule.getBuffer(), tail)) {
											headresults_vec.push_back(*currRule.getHeadconstant());
										}
									}
								}
								else {
									if (currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
										rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, headresults_vec, false);
#pragma omp critical
										{
											if(!currRule.isBuffered()) currRule.setBuffer(headresults_vec);
										}
									}
									else if (currRule.getRuletype() == Ruletype::YRule) {
										std::vector<int> comp;
										rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, comp, false);
#pragma omp critical
										{
											if (!currRule.isBuffered())currRule.setBuffer(headresults_vec);
										}
										if (in_sorted(comp, tail)) {
											headresults_vec.push_back(*currRule.getHeadconstant());
										}
									}
								}
							}

							if (headresults_vec.size() > 0) {
								for (auto headresult : headresults_vec) {
									if (cluster_result_head[headresult] == 0.0) {
										cluster_result_head[headresult] = currRule.getAppliedConfidence();
										touched_cluster_heads.push_back(headresult);
									}
									else {
										if (cluster_result_head[headresult] < currRule.getAppliedConfidence()) {
											cluster_result_head[headresult] = currRule.getAppliedConfidence();
										}
									}
								}
							}
						}
						for (auto i : touched_cluster_heads) {
							if (result_head[i] == 0.0) {
								touched_heads.push_back(i);
							}
							result_head[i] = 1.0 - (1.0 - result_head[i]) * (1.0 - cluster_result_head[i]);
							cluster_result_head[i] = 0.0;
						}
					}

					for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
						int head = v_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];

						MinHeap heads(10);
						for (auto i : touched_heads) {
							if (result_head[i] >= heads.getMin().second) {
								if (i == head || tails->second.find(i) == tails->second.end()) {
									heads.deleteMin();
									heads.insertKey(std::make_pair(i, result_head[i]));
								}
							}
							result_head[i] = 0.0;
						}

						std::vector<std::pair<int,double>> headresults_vec;
						for (int i = 9; i >= 0; i--) {
							std::pair<int, double> head_pred = heads.extractMin();
							if (head_pred.first != 1) headresults_vec.push_back(head_pred);
						}
						std::reverse(headresults_vec.begin(), headresults_vec.end());

						int size_heads = headresults_vec.size() > 10 ? 10 : headresults_vec.size();
						int rank;
						for (int i = size_heads - 1; i >= 0; i--) {
							if (i == size_heads - 1 or headresults_vec[i].second != headresults_vec[i + 1].second) {
								rank = i;
							}
							if (head == headresults_vec[i].first) {
								if (rank == 0) {
									hit1++;
								}
								if (rank < 3) {
									hit3++;
								}
								hit10++;
							}
						}
						predicted++;
					}
				}
				tails++;
			}
		}
		delete[] cluster_result_head;
		delete[] cluster_result_tail;
		delete[] result_head;
		delete[] result_tail;
		return std::make_tuple((double)hit1 / (double)predicted, (double)hit3 / (double)predicted, (double)hit10 / (double)predicted);
	}

	std::tuple<double, double, double> max() {

		int* adj_lists = graph->getCSR()->getAdjList();
		int* adj_list_starts = graph->getCSR()->getAdjBegin();

		Rule* rules_adj_list = rr->getCSR()->getAdjList();
		int* adj_begin = rr->getCSR()->getAdjBegin();

		int** testtriples = ttr->getTesttriples();
		int* testtriplessize = ttr->getTesttriplesSize();

		int* tt_adj_list = ttr->getCSR()->getAdjList();
		int* tt_adj_begin = ttr->getCSR()->getAdjBegin();

		int* vt_adj_list = vtr->getCSR()->getAdjList();
		int* vt_adj_begin = vtr->getCSR()->getAdjBegin();


		int rel = this->relation;
		int nodesize = index->getNodeSize();

		int ind_ptr = adj_begin[3 + rel];
		int lenRules = adj_begin[3 + rel + 1] - ind_ptr;

		{
			// adj list of testtriple x r ?
			int* v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2]]);
			int lenHeads = v_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
			auto heads = vtr->getRelHeadToTails()[rel].begin();
			while (heads != vtr->getRelHeadToTails()[rel].end()) {
				int head = heads->first;
				int* head_ind_ptr = &v_adj_list[3 + head];
				int lenTails = heads->second.size();


				if (lenTails > 0) {
					ScoreTree* tailScoreTrees = new ScoreTree[lenTails];
					std::vector<bool> fineScoreTrees(lenTails);
					bool stop = false;
					for (auto ruleIndex : clusters[0]) {
						Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];
						std::vector<int> tailresults_vec;

						if (currRule.is_c()) {
							rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, false);
						}
						else {
							if (currRule.isBuffered()) {
								if (currRule.getRuletype() == Ruletype::XRule) {
									if (in_sorted(currRule.getBuffer(), head)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else {
									tailresults_vec = currRule.getBuffer();
								}
							}
							else {
								if (currRule.getRuletype() == Ruletype::XRule) {
									std::vector<int> comp;
									rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, comp, false);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(comp);
									}
									if (in_sorted(comp, head)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, tailresults_vec, false);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(tailresults_vec);
									}
								}
							}
						}
						if (tailresults_vec.size() > 0) {
							stop = true;
							for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
								if (fineScoreTrees[tailIndex] == false) {
									int tail = v_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
									std::vector<int> filtered_testresults_vec;
									for (auto a : tailresults_vec) {
										if (a == tail || heads->second.find(a) == heads->second.end()) {
											filtered_testresults_vec.push_back(a);
										}
									}
									tailScoreTrees[tailIndex].addValues(currRule.getAppliedConfidence(), &filtered_testresults_vec[0], filtered_testresults_vec.size());
									if (tailScoreTrees[tailIndex].fine()) {
										fineScoreTrees[tailIndex] = true;
									}
									else {
										stop = false;
									}
								}
							}
						}
						if (stop) {
							break;
						}
					}

					for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
						int tail = v_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
						auto cmp = [](std::pair<int, double> const& a, std::pair<int, double> const& b)
						{
							return a.second > b.second;
						};

						// Get Tailresults and final sorting
						std::vector<std::pair<int, double>> tailresults_vec;
						tailScoreTrees[tailIndex].getResults(tailresults_vec);
						std::sort(tailresults_vec.begin(), tailresults_vec.end(), finalResultComperator);

						int size_tails = tailresults_vec.size() > 10 ? 10 : tailresults_vec.size();
						int rank;
						for (int i = size_tails - 1; i >= 0; i--) {
							if (i == size_tails - 1 or tailresults_vec[i].second != tailresults_vec[i + 1].second) {
								rank = i;
							}
							if (tail == tailresults_vec[i].first) {
								if (rank == 0) {
									hit1++;
								}
								if (rank < 3) {
									hit3++;
								}
								hit10++;
							}
						}
						predicted++;

						tailScoreTrees[tailIndex].Free();
					}
					delete[] tailScoreTrees;
				}
				heads++;
			}
		}
		{
			// adj list of testtriple x r ?
			int* v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2 + 1]]);
			int lenTails = v_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

			auto tails = vtr->getRelTailToHeads()[rel].begin();
			while (tails != vtr->getRelTailToHeads()[rel].end()) {
				int tail = tails->first;
				int* tail_ind_ptr = &v_adj_list[3 + tail];
				int lenHeads = tails->second.size();

				if (lenHeads > 0) {
					ScoreTree* headScoreTrees = new ScoreTree[lenHeads];
					std::vector<bool> fineScoreTrees(lenHeads);
					bool stop = false;
					for (auto ruleIndex : clusters[0]) {
						Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];

						std::vector<int> headresults_vec;

						if (currRule.is_c()) {
							rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, false);
						}
						else {
							if (currRule.isBuffered()) {
								if (currRule.getRuletype() == Ruletype::XRule) {
									headresults_vec = currRule.getBuffer();
								}
								else {
									if (in_sorted(currRule.getBuffer(), tail)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
							}
							else {
								if (currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, headresults_vec, false);
#pragma omp critical
									{
										if(!currRule.isBuffered())currRule.setBuffer(headresults_vec);
									}
								}
								else if (currRule.getRuletype() == Ruletype::YRule) {
									std::vector<int> comp;
									rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, comp, false);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(headresults_vec);
									}
									if (in_sorted(comp, tail)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
							}
						}

						if (headresults_vec.size() > 0) {
							stop = true;
							for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
								if (fineScoreTrees[headIndex] == false) {
									int head = v_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
									std::vector<int> filtered_headresults_vec;
									for (auto a : headresults_vec) {
										if (a == head || tails->second.find(a) == tails->second.end()) {
											filtered_headresults_vec.push_back(a);
										}
									}
									headScoreTrees[headIndex].addValues(currRule.getAppliedConfidence(), &filtered_headresults_vec[0], filtered_headresults_vec.size());
									if (headScoreTrees[headIndex].fine()) {
										fineScoreTrees[headIndex] = true;
									}
									else {
										stop = false;
									}
								}
							}
						}
						if (stop) {
							break;
						}
					}
					for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
						int head = v_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
						// Get Headresults and final sorting
						std::vector<std::pair<int, double>> headresults_vec;
						headScoreTrees[headIndex].getResults(headresults_vec);
						std::sort(headresults_vec.begin(), headresults_vec.end(), finalResultComperator);
						

						int size_heads = headresults_vec.size() > 10 ? 10 : headresults_vec.size();
						int rank;
						for (int i = size_heads - 1; i >= 0; i--) {
							if (i == size_heads - 1 or headresults_vec[i].second != headresults_vec[i + 1].second) {
								rank = i;
							}
							if (head == headresults_vec[i].first) {
								if (rank == 0) {
									hit1++;
								}
								if (rank < 3) {
									hit3++;
								}
								hit10++;
							}
						}
						predicted++;
						headScoreTrees[headIndex].Free();
					}
					delete[] headScoreTrees;
				}
				tails++;
			}
		}

		return std::make_tuple((double)hit1 / (double)predicted, (double)hit3 / (double)predicted, (double)hit10 / (double)predicted);
	}

	bool in_sorted(std::vector<int>& vector, int ele) {
		if (vector.size() == 0) return false;
		int a = 0;
		int b = vector.size() / 2;
		int c = vector.size() - 1;


		if (vector[a] > ele) return false;
		if (vector[c] == ele) return true;
		if (vector[c] < ele) return false;
		int asdf = 0;

		while (true) {
			asdf++;
			if (asdf == 100) {
				std::cout << "INF " << ele << "\n";
				for (auto i : vector) {
					std::cout << i << " ";
				}
				exit(-1);
			}
			if (vector[b] == ele) return true;
			if (a == b or b == c) {
				break;
			}
			if (vector[a] <= ele and ele < vector[b]) {
				c = b;
				b = (a + b) / 2;
			}
			else if (vector[b] < ele and ele <= vector[c]) {
				a = b;
				b = (b + c) / 2;
			}
		}
		return false;
	}

	double get_hit1() {
		return (double)hit1 / (double)predicted;
	}

	double get_hit3() {
		return (double)hit3 / (double)predicted;
	}

	double get_hit10() {
		return (double)hit10 / (double)predicted;
	}

private:
	int relation;

	int hit1 = 0;
	int hit3 = 0;
	int hit10 = 0;
	int predicted = 0;

	std::vector<std::vector<int>> clusters;

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

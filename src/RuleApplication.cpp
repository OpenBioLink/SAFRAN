#include "RuleApplication.h"

RuleApplication::RuleApplication(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->index = index;
	this->graph = graph;
	this->ttr = ttr;
	this->vtr = vtr;
	this->rr = rr;
	this->rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
	reflexiv_token = *index->getIdOfNodestring(Properties::get().REFLEXIV_TOKEN);
}

void RuleApplication::apply_nr_noisy(std::unordered_map<int, std::pair<bool, std::vector<std::vector<int>>>> rel2clusters) {
	fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");
	int iterations = index->getRelSize();
#pragma omp parallel for schedule(dynamic)
	for (int rel = 0; rel < iterations; rel++) {
		if (iterations > 100 and (rel % ((iterations - 1) / 100)) == 0) {
			util::printProgress((double)rel / (double)(iterations - 1));
		}
		std::pair<double, std::vector<std::vector<int>>> cluster = rel2clusters[rel];
		if (cluster.second.size() == 0) {
			continue;
		}

		if (cluster.first == true) {
			max(rel, cluster.second);
		}
		else {
			noisy(rel, cluster.second);
		}
	}
	fclose(pFile);
}

void RuleApplication::apply_only_noisy() {
	fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");

	int* adj_begin = rr->getCSR()->getAdjBegin();
	Rule* rules_adj_list = rr->getCSR()->getAdjList();

	int iterations = index->getRelSize();
#pragma omp parallel for schedule(dynamic)
	for (int rel = 0; rel < iterations; rel++) {
		if (iterations > 100 and (rel % ((iterations - 1) / 100)) == 0) {
			util::printProgress((double)rel / (double)(iterations - 1));
		}
		int ind_ptr = adj_begin[3 + rel];
		int lenRules = adj_begin[3 + rel + 1] - ind_ptr;
		std::vector<std::vector<int>> clusters;
		for (int i = 0; i < lenRules; i++) {
			std::vector<int> cluster;
			Rule& r = rules_adj_list[ind_ptr + i];
			cluster.push_back(i);
			clusters.push_back(cluster);
		}
		noisy(rel, clusters);
	}
	fclose(pFile);
}

void RuleApplication::apply_only_max() {
	fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");
	int* adj_begin = rr->getCSR()->getAdjBegin();
	Rule* rules_adj_list = rr->getCSR()->getAdjList();

	int iterations = index->getRelSize();
#pragma omp parallel for schedule(dynamic)
	for (int rel = 0; rel < iterations; rel++) {
		if (iterations > 100 and (rel % ((iterations - 1) / 100)) == 0) {
			util::printProgress((double)rel / (double)(iterations - 1));
		}
		int ind_ptr = adj_begin[3 + rel];
		int lenRules = adj_begin[3 + rel + 1] - ind_ptr;
		std::vector<std::vector<int>> clusters;
		std::vector<int> cluster;
		for (int i = 0; i < lenRules; i++) {
			Rule& r = rules_adj_list[ind_ptr + i];
			cluster.push_back(i);
		}
		clusters.push_back(cluster);
		max(rel, clusters);
	}
	fclose(pFile);
}

void RuleApplication::noisy(int rel, std::vector<std::vector<int>> clusters) {

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

	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> headTailResults;
	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> tailHeadResults;

	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2]]);
		int lenHeads = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
		auto heads = ttr->getRelHeadToTails()[rel].begin();
		while (heads != ttr->getRelHeadToTails()[rel].end()) {
			int head = heads->first;
			int* head_ind_ptr = &t_adj_list[3 + head];
			int lenTails = heads->second.size();


			if (lenTails > 0) {
				std::vector<int> touched_tails;
				for (int i = 0; i < clusters.size(); i++) {

					std::vector<int> touched_cluster_tails;

					for (auto ruleIndex : clusters[i]) {
						Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];

						std::vector<int> tailresults_vec;

						if (currRule.is_c()) {
							rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, true, true);
						}
						else {

							if (currRule.isBuffered()) {
								if (currRule.getRuletype() == Ruletype::XRule) {
									if (util::in_sorted(currRule.getBuffer(), head)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
									tailresults_vec = currRule.getBuffer();
								}
							}
							else {
								if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::XRule) {
									std::vector<int> comp;
									rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, comp, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(comp);
									}
									if (util::in_sorted(comp, head)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, tailresults_vec, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(tailresults_vec);
									}
								}else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::XRule) {
									std::vector<int> comp;
									rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, comp, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(comp);
									}
									if (util::in_sorted(comp, head)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
									rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, tailresults_vec, true, true);
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
					int tail = t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];

					MinHeap tails(10);
					for (auto i : touched_tails) {
						if (result_tail[i] >= tails.getMin().second) {
							if (i == reflexiv_token) {
								i = tail;
							}
							if (i == tail || heads->second.find(i) == heads->second.end()) {
								tails.deleteMin();
								tails.insertKey(std::make_pair(i, result_tail[i]));
							}
						}
					}

					std::vector<std::pair<int, double>> tailresults_vec;
					for (int i = 9; i >= 0; i--) {
						std::pair<int, double> tail_pred = tails.extractMin();
						if (tail_pred.first != -1) tailresults_vec.push_back(tail_pred);
					}
					std::reverse(tailresults_vec.begin(), tailresults_vec.end());
					headTailResults[head][tail] = tailresults_vec;

				}
				for (auto i : touched_tails) {
					result_tail[i] = 0.0;
				}
			}
			heads++;
		}
	}
	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2 + 1]]);
		int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

		auto tails = ttr->getRelTailToHeads()[rel].begin();
		while (tails != ttr->getRelTailToHeads()[rel].end()) {
			int tail = tails->first;
			int* tail_ind_ptr = &t_adj_list[3 + tail];
			int lenHeads = tails->second.size();


			if (lenHeads > 0) {
				std::vector<int> touched_heads;
				for (int i = 0; i < clusters.size(); i++) {

					std::vector<int> touched_cluster_heads;

					for (auto ruleIndex : clusters[i]) {
						Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];

						std::vector<int> headresults_vec;

						if (currRule.is_c()) {
							rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, true, true);
						}
						else {
							if (currRule.isBuffered()) {
								if (currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
									headresults_vec = currRule.getBuffer();
								}
								else if (currRule.getRuletype() == Ruletype::YRule) {
									if (util::in_sorted(currRule.getBuffer(), tail)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
							}
							else {
								if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, headresults_vec, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered()) currRule.setBuffer(headresults_vec);
									}
								}
								else if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::YRule) {
									std::vector<int> comp;
									rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, comp, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(headresults_vec);
									}
									if (util::in_sorted(comp, tail)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								} else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
									rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, headresults_vec, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered()) currRule.setBuffer(headresults_vec);
									}
								}
								else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::YRule) {
									std::vector<int> comp;
									rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, comp, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(headresults_vec);
									}
									if (util::in_sorted(comp, tail)) {
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
					int head = t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];

					MinHeap heads(10);
					for (auto i : touched_heads) {
						if (result_head[i] >= heads.getMin().second) {
							if (i == reflexiv_token) {
								i = head;
							}
							if (i == head || tails->second.find(i) == tails->second.end()) {
								heads.deleteMin();
								heads.insertKey(std::make_pair(i, result_head[i]));
							}
						}
					}

					std::vector<std::pair<int, double>> headresults_vec;
					for (int i = 9; i >= 0; i--) {
						std::pair<int, double> head_pred = heads.extractMin();
						if (head_pred.first != -1) headresults_vec.push_back(head_pred);
					}
					std::reverse(headresults_vec.begin(), headresults_vec.end());
					tailHeadResults[tail][head] = headresults_vec;
				}
				for (auto i : touched_heads) {
					result_head[i] = 0.0;
				}
			}
			tails++;
		}
	}
	auto it_head = headTailResults.begin();
	while (it_head != headTailResults.end()) {
		auto it_tail = it_head->second.begin();
		while (it_tail != it_head->second.end()) {
			#pragma omp critical
			{
				writeTopKCandidates(it_head->first, rel, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, Properties::get().TOP_K_OUTPUT);
			}
			it_tail++;
		}
		it_head++;
	}
	delete[] cluster_result_head;
	delete[] cluster_result_tail;
	delete[] result_head;
	delete[] result_tail;
}

void RuleApplication::max(int rel, std::vector<std::vector<int>> clusters) {

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

	int nodesize = index->getNodeSize();

	int ind_ptr = adj_begin[3 + rel];
	int lenRules = adj_begin[3 + rel + 1] - ind_ptr;


	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> headTailResults;
	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> tailHeadResults;

	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2]]);
		int lenHeads = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
		auto heads = ttr->getRelHeadToTails()[rel].begin();
		while (heads != ttr->getRelHeadToTails()[rel].end()) {
			int head = heads->first;
			int* head_ind_ptr = &t_adj_list[3 + head];
			int lenTails = heads->second.size();


			if (lenTails > 0) {
				ScoreTree* tailScoreTrees = new ScoreTree[lenTails];
				std::vector<bool> fineScoreTrees(lenTails);
				bool stop = false;
				for (auto ruleIndex : clusters[0]) {
					Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];
					std::vector<int> tailresults_vec;

					if (currRule.is_c()) {
						rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, true, false);
					}
					else {
							if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::XRule) {
								if (rulegraph->existsAcyclic(&head, currRule, true)) {
									tailresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
							else if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
								rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, tailresults_vec, true, false);
							} else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::XRule) {
								if (rulegraph->existsAcyclic(&head, currRule, true)) {
									tailresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
							else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
								rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, tailresults_vec, true, false);
							}
					}
					if (tailresults_vec.size() > 0) {
						stop = true;
						for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
							if (fineScoreTrees[tailIndex] == false) {
								int tail = t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
								std::vector<int> filtered_testresults_vec;
								for (auto a : tailresults_vec) {
									if (a == reflexiv_token) {
										a = tail;
									}
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
					int tail = t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
					auto cmp = [](std::pair<int, double> const& a, std::pair<int, double> const& b)
					{
						return a.second > b.second;
					};

					// Get Tailresults and final sorting
					std::vector<std::pair<int, double>> tailresults_vec;
					tailScoreTrees[tailIndex].getResults(tailresults_vec);
					std::sort(tailresults_vec.begin(), tailresults_vec.end(), finalResultComperator);

					headTailResults[head][tail] = tailresults_vec;

					tailScoreTrees[tailIndex].Free();
				}
				delete[] tailScoreTrees;
			}
			heads++;
		}
	}
	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2 + 1]]);
		int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

		auto tails = ttr->getRelTailToHeads()[rel].begin();
		while (tails != ttr->getRelTailToHeads()[rel].end()) {
			int tail = tails->first;
			int* tail_ind_ptr = &t_adj_list[3 + tail];
			int lenHeads = tails->second.size();

			if (lenHeads > 0) {
				ScoreTree* headScoreTrees = new ScoreTree[lenHeads];
				std::vector<bool> fineScoreTrees(lenHeads);
				bool stop = false;
				for (auto ruleIndex : clusters[0]) {
					Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];

					std::vector<int> headresults_vec;

					if (currRule.is_c()) {
						rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, true, false);
					}
					else {
							if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
								rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, headresults_vec, true, false);
							}
							else if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::YRule) {
								if (rulegraph->existsAcyclic(&tail, currRule, true)) {
									headresults_vec.push_back(*currRule.getHeadconstant());
								}
							} else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
								rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, headresults_vec, true, false);
							}
							else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::YRule) {
								if (rulegraph->existsAcyclic(&tail, currRule, true)) {
									headresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
					}

					if (headresults_vec.size() > 0) {
						stop = true;
						for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
							if (fineScoreTrees[headIndex] == false) {
								int head = t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
								std::vector<int> filtered_headresults_vec;
								for (auto a : headresults_vec) {
									if (a == reflexiv_token) {
										a = head;
									}
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
					int head = t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
					// Get Headresults and final sorting
					std::vector<std::pair<int, double>> headresults_vec;
					headScoreTrees[headIndex].getResults(headresults_vec);
					std::sort(headresults_vec.begin(), headresults_vec.end(), finalResultComperator);

					tailHeadResults[tail][head] = headresults_vec;

					headScoreTrees[headIndex].Free();
				}
				delete[] headScoreTrees;
			}
			tails++;
		}
	}

	auto it_head = headTailResults.begin();
	while (it_head != headTailResults.end()) {
		auto it_tail = it_head->second.begin();
		while (it_tail != it_head->second.end()) {
			#pragma omp critical
			{
				writeTopKCandidates(it_head->first, rel, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, Properties::get().TOP_K_OUTPUT);
			}
			it_tail++;
		}
		it_head++;
	}
}

	

void RuleApplication::writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, double>> headresults, std::vector<std::pair<int, double>> tailresults, FILE* pFile, int& K) {
	fprintf(pFile, "%s %s %s\nHeads: ", index->getStringOfNodeId(head)->c_str(), index->getStringOfRelId(rel)->c_str(), index->getStringOfNodeId(tail)->c_str());

	int maxHead = headresults.size() < K ? headresults.size() : K;
	for (int i = 0; i < maxHead; i++) {
		fprintf(pFile, "%s\t%.16f\t", index->getStringOfNodeId(headresults[i].first)->c_str(), headresults[i].second);
	}
	fprintf(pFile, "\nTails: ");
	int maxTail = tailresults.size() < K ? tailresults.size() : K;
	for (int i = 0; i < maxTail; i++) {
		fprintf(pFile, "%s\t%.16f\t", index->getStringOfNodeId(tailresults[i].first)->c_str(), tailresults[i].second);
	}
	fprintf(pFile, "\n");
}

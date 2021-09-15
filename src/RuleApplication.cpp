#include "RuleApplication.h"

RuleApplication::RuleApplication(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->index = index;
	this->graph = graph;
	this->ttr = ttr;
	this->vtr = vtr;
	this->rr = rr;
	this->rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
	reflexiv_token = *index->getIdOfNodestring(Properties::get().REFLEXIV_TOKEN);
	this->k = Properties::get().TOP_K_OUTPUT;
}

void RuleApplication::apply_nr_noisy(std::unordered_map<int, std::pair<std::pair<bool, std::vector<std::vector<int>>>, std::pair<bool, std::vector<std::vector<int>>>>> rel2clusters) {

	int* adj_begin = rr->getCSR()->getAdjBegin();
	Rule* rules_adj_list = rr->getCSR()->getAdjList();

	fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");
	int iterations = index->getRelSize();
	for (int rel = 0; rel < iterations; rel++) {
		if (iterations > 100 && (rel % ((iterations - 1) / 100)) == 0) {
			util::printProgress((double)rel / (double)(iterations - 1));
		}
		else {
			std::cout << "Relation " << rel << "/" << iterations << " " << *index->getStringOfRelId(rel) << "\n";
		}
		std::pair<std::pair<bool, std::vector<std::vector<int>>>, std::pair<bool, std::vector<std::vector<int>>>> cluster = rel2clusters[rel];


		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> headTailResults;
		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> tailHeadResults;

		std::cout << cluster.first.first << " " << cluster.first.second.size() << " " << cluster.second.first << " " << cluster.second.second.size() << "\n";
		if (cluster.first.first == true) {
			tailHeadResults = max(rel, cluster.first.second, true);
		}
		else {
			tailHeadResults = noisy(rel, cluster.first.second, true);
		}

		if (cluster.second.first == true) {
			headTailResults = max(rel, cluster.second.second, false);
		}
		else {
			headTailResults = noisy(rel, cluster.second.second, false);
		}

		auto it_head = headTailResults.begin();
		while (it_head != headTailResults.end()) {
			auto it_tail = it_head->second.begin();
			while (it_tail != it_head->second.end()) {
				{
					writeTopKCandidates(it_head->first, rel, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, k);
				}
				it_tail++;
			}
			it_head++;
		}
	}
	fclose(pFile);
}

void RuleApplication::apply_only_noisy() {
	fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");

	int* adj_begin = rr->getCSR()->getAdjBegin();
	Rule* rules_adj_list = rr->getCSR()->getAdjList();

	int iterations = index->getRelSize();
	for (int rel = 0; rel < iterations; rel++) {
		if (iterations > 100 && (rel % ((iterations - 1) / 100)) == 0) {
			util::printProgress((double)rel / (double)(iterations - 1));
		}
		else {
			std::cout << "Relation " << rel << "/" << iterations << " " << *index->getStringOfRelId(rel) << "\n";
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

		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> headTailResults;
		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> tailHeadResults;
		tailHeadResults = noisy(rel, clusters, true);
		headTailResults = noisy(rel, clusters, false);
		auto it_head = headTailResults.begin();
		while (it_head != headTailResults.end()) {
			auto it_tail = it_head->second.begin();
			while (it_tail != it_head->second.end()) {
				{
					writeTopKCandidates(it_head->first, rel, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, k);
				}
				it_tail++;
			}
			it_head++;
		}
	}
	fclose(pFile);
}

void RuleApplication::apply_only_max() {
	fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");
	int* adj_begin = rr->getCSR()->getAdjBegin();
	Rule* rules_adj_list = rr->getCSR()->getAdjList();

	int iterations = index->getRelSize();
	for (int rel = 0; rel < iterations; rel++) {
		if (iterations > 100 && (rel % ((iterations - 1) / 100)) == 0) {
			util::printProgress((double)rel / (double)(iterations - 1));
		}
		else {
			std::cout << "Relation " << rel << "/" << iterations << " " << *index->getStringOfRelId(rel) << "\n";
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

		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> headTailResults;
		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> tailHeadResults;
		tailHeadResults = max(rel, clusters, true);
		headTailResults = max(rel, clusters, false);
		auto it_head = headTailResults.begin();
		while (it_head != headTailResults.end()) {
			auto it_tail = it_head->second.begin();
			while (it_tail != it_head->second.end()) {
				{
					writeTopKCandidates(it_head->first, rel, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, k);
				}
				it_tail++;
			}
			it_head++;
		}
	}
	fclose(pFile);
}

/*
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
		auto rHT = ttr->getRelHeadToTails()[rel];
#pragma omp parallel for schedule(dynamic)
		for (int b = 0; b < rHT.bucket_count(); b++) {
			double* result_tail = new double[nodesize];
			int * count_tail = new int[nodesize];
			std::fill(result_tail, result_tail + nodesize, 0.0);
			std::fill(count_tail, count_tail + nodesize, 0);
			for (auto heads = rHT.begin(b); heads != rHT.end(b); heads++) {
				int head = heads->first;
				int* head_ind_ptr = &t_adj_list[3 + head];
				int lenTails = heads->second.size();


				if (lenTails > 0) {
					std::vector<int> touched_tails;

					for (int ruleIndex = 0; ruleIndex < lenRules; ruleIndex++) {
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
								else if (currRule.getRuletype() == Ruletype::YRule && head == *currRule.getHeadconstant()) {
									tailresults_vec = currRule.getBuffer();
								}
							}
							else {
								if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::XRule) {
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
								else if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::YRule && head == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, tailresults_vec, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(tailresults_vec);
									}
								}
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::XRule) {
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
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::YRule && head == *currRule.getHeadconstant()) {
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
								if (result_tail[tailresult] == 0.0) {
									result_tail[tailresult] = currRule.getAppliedConfidence();
									count_tail[tailresult]++;
									touched_tails.push_back(tailresult);
								}
								else {
									if (result_tail[tailresult] < currRule.getAppliedConfidence()) {
										throw std::runtime_error("HI");
									}
									else {
										result_tail[tailresult] = result_tail[tailresult] + (pow(0.0001,(double)count_tail[tailresult]) * currRule.getAppliedConfidence());
										count_tail[tailresult]++;
									}
								}
							}
						}
					}

					for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
						int tail = t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];

						MinHeap tails(k);
						for (auto i : touched_tails) {
							if (result_tail[i] >= tails.getMin().second) {
								double confidence = result_tail[i];
								if (i == reflexiv_token) {
									i = head;
								}
								if (i == tail || heads->second.find(i) == heads->second.end()) {
									tails.deleteMin();
									tails.insertKey(std::make_pair(i, confidence));
								}
							}
						}

						std::vector<std::pair<int, double>> tailresults_vec;
						for (int i = k - 1; i >= 0; i--) {
							std::pair<int, double> tail_pred = tails.extractMin();
							if (tail_pred.first != -1) tailresults_vec.push_back(tail_pred);
						}
						std::reverse(tailresults_vec.begin(), tailresults_vec.end());
#pragma omp critical
						{
							headTailResults[head][tail] = tailresults_vec;
						}

					}
					for (auto i : touched_tails) {
						result_tail[i] = 0.0;
						count_tail[i] = 0;
					}
				}
			}
			delete[] count_tail;
			delete[] result_tail;
		}
	}
	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2 + 1]]);
		int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
		auto rTH = ttr->getRelTailToHeads()[rel];
#pragma omp parallel for schedule(dynamic)
		for (int b = 0; b < rTH.bucket_count(); b++) {
			double* result_head = new double[nodesize];
			int* count_head = new int[nodesize];
			std::fill(result_head, result_head + nodesize, 0.0);
			std::fill(count_head, count_head + nodesize, 0);

			for (auto tails = rTH.begin(b); tails != rTH.end(b); tails++) {
				int tail = tails->first;
				int* tail_ind_ptr = &t_adj_list[3 + tail];
				int lenHeads = tails->second.size();


				if (lenHeads > 0) {
					std::vector<int> touched_heads;

					for (auto ruleIndex : clusters[0]) {
						Rule& currRule = rules_adj_list[ind_ptr + ruleIndex];

						std::vector<int> headresults_vec;

						if (currRule.is_c()) {
							rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, true, true);
						}
						else {
							if (currRule.isBuffered()) {
								if (currRule.getRuletype() == Ruletype::XRule && tail == *currRule.getHeadconstant()) {
									headresults_vec = currRule.getBuffer();
								}
								else if (currRule.getRuletype() == Ruletype::YRule) {
									if (util::in_sorted(currRule.getBuffer(), tail)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
							}
							else {
								if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::XRule && tail == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, headresults_vec, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered()) currRule.setBuffer(headresults_vec);
									}
								}
								else if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::YRule) {
									std::vector<int> comp;
									rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, comp, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(comp);
									}
									if (util::in_sorted(comp, tail)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::XRule && tail == *currRule.getHeadconstant()) {
									rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, headresults_vec, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered()) currRule.setBuffer(headresults_vec);
									}
								}
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::YRule) {
									std::vector<int> comp;
									rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, comp, true, true);
#pragma omp critical
									{
										if (!currRule.isBuffered())currRule.setBuffer(comp);
									}
									if (util::in_sorted(comp, tail)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
							}
						}

						if (headresults_vec.size() > 0) {
							for (auto headresult : headresults_vec) {
								if (result_head[headresult] == 0.0) {
									result_head[headresult] = currRule.getAppliedConfidence();
									touched_heads.push_back(headresult);
									count_head[headresult]++;
								}
								else {
									if (result_head[headresult] < currRule.getAppliedConfidence()) {
										throw std::runtime_error("HI");
									}
									else {
										result_head[headresult] = result_head[headresult] + (pow(0.0001,count_head[headresult]) * currRule.getAppliedConfidence());
										count_head[headresult]++;
									}
								}
							}
						}
					}

					for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
						int head = t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];

						MinHeap heads(k);
						for (auto i : touched_heads) {
							if (result_head[i] >= heads.getMin().second) {
								double confidence = result_head[i];
								if (i == reflexiv_token) {
									i = tail;
								}
								if (i == head || tails->second.find(i) == tails->second.end()) {
									heads.deleteMin();
									heads.insertKey(std::make_pair(i, confidence));
								}
							}
						}

						std::vector<std::pair<int, double>> headresults_vec;
						for (int i = k - 1; i >= 0; i--) {
							std::pair<int, double> head_pred = heads.extractMin();
							if (head_pred.first != -1) headresults_vec.push_back(head_pred);
						}
						std::reverse(headresults_vec.begin(), headresults_vec.end());
#pragma omp critical
						{
							tailHeadResults[tail][head] = headresults_vec;
						}
					}
					for (auto i : touched_heads) {
						result_head[i] = 0.0;
						count_head[i] = 0;
					}
				}
			}
			delete[] count_head;
			delete[] result_head;
		}
	}
#pragma omp critical
	{
		auto it_head = headTailResults.begin();
		while (it_head != headTailResults.end()) {
			auto it_tail = it_head->second.begin();
			while (it_tail != it_head->second.end()) {
				{
					writeTopKCandidates(it_head->first, rel, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, k);
				}
				it_tail++;
			}
			it_head++;
		}
	}
}
*/

std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> RuleApplication::noisy(int rel, std::vector<std::vector<int>> clusters, bool predictHeadNotTail) {
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

	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> entityEntityResults;

	if(!predictHeadNotTail)
	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2]]);
		int lenHeads = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
		auto rHT = ttr->getRelHeadToTails()[rel];
#pragma omp parallel for schedule(dynamic)
		for (int b = 0; b < rHT.bucket_count(); b++) {
			float50* result_tail = new float50[nodesize];
			double* cluster_result_tail = new double[nodesize];
			std::fill(result_tail, result_tail + nodesize, 0.0);
			std::fill(cluster_result_tail, cluster_result_tail + nodesize, 0.0);
			for (auto heads = rHT.begin(b); heads != rHT.end(b); heads++) {
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
								rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, true, false);
							}
							else {
								if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::XRule && *currRule.getHeadconstant() != head) {
									if (rulegraph->existsAcyclic(&head, currRule, true)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::YRule && head == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, tailresults_vec, true, false);
								}
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::XRule && *currRule.getHeadconstant() != head) {
									if (rulegraph->existsAcyclic(&head, currRule, true)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::YRule && head == *currRule.getHeadconstant()) {
									rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, tailresults_vec, true, false);
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

						MinHeap tails(k);
						for (auto i : touched_tails) {
							if (result_tail[i] >= tails.getMin().second) {
								float50 confidence = result_tail[i];
								if (i == head) continue;
								if (i == reflexiv_token) {
									i = head;
								}
								if (i == tail || heads->second.find(i) == heads->second.end()) {
									tails.deleteMin();
									tails.insertKey(std::make_pair(i, confidence));
								}
							}
						}

						std::vector<std::pair<int, float50>> tailresults_vec;
						for (int i = k - 1; i >= 0; i--) {
							std::pair<int, float50> tail_pred = tails.extractMin();
							if (tail_pred.first != -1) tailresults_vec.push_back(tail_pred);
						}
						std::reverse(tailresults_vec.begin(), tailresults_vec.end());
#pragma omp critical
						{
							entityEntityResults[head][tail] = tailresults_vec;
						}

					}
					for (auto i : touched_tails) {
						result_tail[i] = 0.0;
					}
				}
			}
			delete[] result_tail;
			delete[] cluster_result_tail;
		}
	} else {
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2 + 1]]);
		int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
		auto rTH = ttr->getRelTailToHeads()[rel];
#pragma omp parallel for schedule(dynamic)
		for (int b = 0; b < rTH.bucket_count(); b++) {
			float50* result_head = new float50[nodesize];
			double* cluster_result_head = new double[nodesize];
			std::fill(result_head, result_head + nodesize, 0.0);
			std::fill(cluster_result_head, cluster_result_head + nodesize, 0.0);

			for (auto tails = rTH.begin(b); tails != rTH.end(b); tails++) {
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
								rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, true, false);
							}
							else {
								if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::XRule && tail == *currRule.getHeadconstant()) {
									rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, headresults_vec, true, false);
								}
								else if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::YRule && *currRule.getHeadconstant() != tail) {
									if (rulegraph->existsAcyclic(&tail, currRule, true)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::XRule && tail == *currRule.getHeadconstant()) {
									rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, headresults_vec, true, false);
								}
								else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::YRule && *currRule.getHeadconstant() != tail) {
									if (rulegraph->existsAcyclic(&tail, currRule, true)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
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

						MinHeap heads(k);
						for (auto i : touched_heads) {
							if (result_head[i] >= heads.getMin().second) {
								float50 confidence = result_head[i];
								if (i == tail) continue;
								if (i == reflexiv_token) {
									i = tail;
								}
								if (i == head || tails->second.find(i) == tails->second.end()) {
									heads.deleteMin();
									heads.insertKey(std::make_pair(i, confidence));
								}
							}
						}

						std::vector<std::pair<int, float50>> headresults_vec;
						for (int i = k - 1; i >= 0; i--) {
							std::pair<int, float50> head_pred = heads.extractMin();
							if (head_pred.first != -1) headresults_vec.push_back(head_pred);
						}
						std::reverse(headresults_vec.begin(), headresults_vec.end());
#pragma omp critical
						{
							entityEntityResults[tail][head] = headresults_vec;
						}
					}
					for (auto i : touched_heads) {
						result_head[i] = 0.0;
					}
				}
			}
			delete[] result_head;
			delete[] cluster_result_head;
		}
	}
	return entityEntityResults;
}



std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> RuleApplication::max(int rel, std::vector<std::vector<int>> clusters, bool predictHeadNotTail) {

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


	std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, float50>>>> entityEntityResults;

	if (!predictHeadNotTail)
	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2]]);
		int lenHeads = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation
		auto rHT = ttr->getRelHeadToTails()[rel];
#pragma omp parallel for schedule(dynamic)
		for (int b = 0; b < rHT.bucket_count(); b++) {
			for (auto heads = rHT.begin(b); heads != rHT.end(b); heads++) {
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
							if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::XRule && *currRule.getHeadconstant() != head) {
								if (rulegraph->existsAcyclic(&head, currRule, true)) {
									tailresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
							else if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::YRule && head == *currRule.getHeadconstant()) {
								rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, tailresults_vec, true, false);
							}
							else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::XRule && *currRule.getHeadconstant() != head) {
								if (rulegraph->existsAcyclic(&head, currRule, true)) {
									tailresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
							else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::YRule && head == *currRule.getHeadconstant()) {
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
										if (a == head) continue;
										if (a == reflexiv_token) {
											a = head;
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

						// convert to float50 hack
						std::vector<std::pair<int, float50>> tailresults_vec_float50;
						for (auto a : tailresults_vec) {
							tailresults_vec_float50.push_back(std::make_pair(a.first, (float50)a.second));
						}

#pragma omp critical
						{
							entityEntityResults[head][tail] = tailresults_vec_float50;
						}

						tailScoreTrees[tailIndex].Free();
					}
					delete[] tailScoreTrees;
				}
			}
		}
	} else
	{
		// adj list of testtriple x r ?
		int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2 + 1]]);
		int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

		auto rTH = ttr->getRelTailToHeads()[rel];
#pragma omp parallel for schedule(dynamic)
		for (int b = 0; b < rTH.bucket_count(); b++) {
			for (auto tails = rTH.begin(b); tails != rTH.end(b); tails++) {
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
							if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::XRule && tail == *currRule.getHeadconstant()) {
								rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, headresults_vec, true, false);
							}
							else if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::YRule && *currRule.getHeadconstant() != tail) {
								if (rulegraph->existsAcyclic(&tail, currRule, true)) {
									headresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
							else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::XRule && tail == *currRule.getHeadconstant()) {
								rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, headresults_vec, true, false);
							}
							else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::YRule && *currRule.getHeadconstant() != tail) {
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
										if (a == tail) continue;
										if (a == reflexiv_token) {
											a = tail;
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


						// convert to float50 hack
						std::vector<std::pair<int, float50>> headresults_vec_float50;
						for (auto a : headresults_vec) {
							headresults_vec_float50.push_back(std::make_pair(a.first, (float50)a.second));
						}

#pragma omp critical
						{
							entityEntityResults[tail][head] = headresults_vec_float50;
						}

						headScoreTrees[headIndex].Free();
					}
					delete[] headScoreTrees;
				}
			}
		}
	}
	return entityEntityResults;
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

void RuleApplication::writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, float50>> headresults, std::vector<std::pair<int, float50>> tailresults, FILE* pFile, int& K) {
	fprintf(pFile, "%s %s %s\nHeads: ", index->getStringOfNodeId(head)->c_str(), index->getStringOfRelId(rel)->c_str(), index->getStringOfNodeId(tail)->c_str());

	int maxHead = headresults.size() < K ? headresults.size() : K;
	for (int i = 0; i < maxHead; i++) {
		fprintf(pFile, "%s\t%s\t", index->getStringOfNodeId(headresults[i].first)->c_str(), headresults[i].second.str().c_str());
	}
	fprintf(pFile, "\nTails: ");
	int maxTail = tailresults.size() < K ? tailresults.size() : K;
	for (int i = 0; i < maxTail; i++) {
		fprintf(pFile, "%s\t%s\t", index->getStringOfNodeId(tailresults[i].first)->c_str(), tailresults[i].second.str().c_str());
	}
	fprintf(pFile, "\n");
}

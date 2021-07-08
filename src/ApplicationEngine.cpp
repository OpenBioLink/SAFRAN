#include "ApplicationEngine.h"

ApplicationEngine::ApplicationEngine(int relation, RuleGraph* rulegraph, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->relation = relation;
	this->index = index;
	this->graph = graph;
	this->ttr = ttr;
	this->rr = rr;
	this->vtr = vtr;
	this->rulegraph = rulegraph;
	reflexiv_token = *index->getIdOfNodestring(Properties::get().REFLEXIV_TOKEN);
	this->k = Properties::get().TOP_K_OUTPUT;
}

std::pair<double,double> ApplicationEngine::noisy(std::vector<std::vector<int>> clusters) {
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

	float50* result_head = new float50[nodesize];
	float50* result_tail = new float50[nodesize];
	double* cluster_result_head = new double[nodesize];
	double* cluster_result_tail = new double[nodesize];
	std::fill(result_head, result_head + nodesize, 0.0);
	std::fill(result_tail, result_tail + nodesize, 0.0);
	std::fill(cluster_result_head, cluster_result_head + nodesize, 0.0);
	std::fill(cluster_result_tail, cluster_result_tail + nodesize, 0.0);

	double mrr_head = 0.0;
	int predicted_head = 0;
	double mrr_tail = 0.0;
	int predicted_tail = 0;

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
							if (currRule.isHeadBuffered(head)) {
								tailresults_vec = currRule.getHeadBuffered(head);
							}
							else {
								rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, false, true);
							}
						}
						else {

							if (currRule.isBuffered()) {
								if (currRule.getRuletype() == Ruletype::XRule and *currRule.getHeadconstant() != head && cluster_result_tail[*currRule.getHeadconstant()] < currRule.getAppliedConfidence()) {
									if (rulegraph->existsAcyclic(&head, currRule, false)) {
										tailresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
								else if(currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()){
									tailresults_vec = currRule.getBuffer();
								}
							}
							else {
								throw new std::runtime_error("NOT BUFFERED");
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
					for (int i = k-1; i >= 0; i--) {
						std::pair<int, float50> tail_pred = tails.extractMin();
						if (tail_pred.first != -1) tailresults_vec.push_back(tail_pred);
					}
					std::reverse(tailresults_vec.begin(), tailresults_vec.end());

					int size_tails = tailresults_vec.size() > k ? k : tailresults_vec.size();
					int rank;
					for (int i = size_tails - 1; i >= 0; i--) {
						if (i == size_tails - 1 or tailresults_vec[i].second != tailresults_vec[i + 1].second) {
							rank = i;
						}
						if (tail == tailresults_vec[i].first) {
							mrr_tail = mrr_tail + (1.0 / (rank + 1));
						}
					}
					predicted_tail++;
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
							if (currRule.isTailBuffered(tail)) {
								headresults_vec = currRule.getTailBuffered(tail);
							}
							else {
								rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, false, true);
							}
						}
						else {
							if (currRule.isBuffered()) {
								if (currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
									headresults_vec = currRule.getBuffer();
								}
								else if(currRule.getRuletype() == Ruletype::YRule and tail != *currRule.getHeadconstant() && cluster_result_head[*currRule.getHeadconstant()] < currRule.getAppliedConfidence()){
									if (rulegraph->existsAcyclic(&tail, currRule, false)) {
										headresults_vec.push_back(*currRule.getHeadconstant());
									}
								}
							}
							else {
								throw new std::runtime_error("NOT BUFFERED");
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
					for (int i = k-1; i >= 0; i--) {
						std::pair<int, float50> head_pred = heads.extractMin();
						if (head_pred.first != 1) headresults_vec.push_back(head_pred);
					}
					std::reverse(headresults_vec.begin(), headresults_vec.end());

					int size_heads = headresults_vec.size() > k ? k : headresults_vec.size();
					int rank;
					for (int i = size_heads - 1; i >= 0; i--) {
						if (i == size_heads - 1 or headresults_vec[i].second != headresults_vec[i + 1].second) {
							rank = i;
						}
						if (head == headresults_vec[i].first) {
							mrr_head = mrr_head + (1.0 / (rank + 1));
						}
					}
					predicted_head++;
				}
				for (auto i : touched_heads) {
					result_head[i] = 0.0;
				}
			}
			tails++;
		}
	}
	delete[] cluster_result_head;
	delete[] cluster_result_tail;
	delete[] result_head;
	delete[] result_tail;
	return std::make_pair(((double)mrr_head / (double)predicted_head), ((double)mrr_tail / (double)predicted_tail));
}

std::pair<double, double> ApplicationEngine::max(std::vector<std::vector<int>> clusters) {

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

	double mrr_head = 0.0;
	int predicted_head = 0;
	double mrr_tail = 0.0;
	int predicted_tail = 0;

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
						if (currRule.isHeadBuffered(head)) {
							tailresults_vec = currRule.getHeadBuffered(head);
						}
						else {
							rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, false, false);
						}
					}
					else {
						if (currRule.isBuffered()) {
							if (currRule.getRuletype() == Ruletype::XRule and head != *currRule.getHeadconstant()) {
								if (rulegraph->existsAcyclic(&head, currRule, false)) {
									tailresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
							else if (currRule.getRuletype() == Ruletype::YRule and head == *currRule.getHeadconstant()) {
								tailresults_vec = currRule.getBuffer();
							}
						}
						else {
							throw new std::runtime_error("NOT BUFFERED");
						}
					}
					if (tailresults_vec.size() > 0) {
						stop = true;
						for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
							if (fineScoreTrees[tailIndex] == false) {
								int tail = v_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
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
					int tail = v_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
					auto cmp = [](std::pair<int, double> const& a, std::pair<int, double> const& b)
					{
						return a.second > b.second;
					};

					// Get Tailresults and final sorting
					std::vector<std::pair<int, double>> tailresults_vec;
					tailScoreTrees[tailIndex].getResults(tailresults_vec);
					std::sort(tailresults_vec.begin(), tailresults_vec.end(), finalResultComperator);

					int size_tails = tailresults_vec.size() > k ? k : tailresults_vec.size();
					int rank;
					for (int i = size_tails - 1; i >= 0; i--) {
						if (i == size_tails - 1 or tailresults_vec[i].second != tailresults_vec[i + 1].second) {
							rank = i;
						}
						if (tail == tailresults_vec[i].first) {
							mrr_tail = mrr_tail + (1.0 / (rank + 1));
						}
					}
					predicted_tail++;

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
						if (currRule.isTailBuffered(tail)) {
							headresults_vec = currRule.getTailBuffered(tail);
						}
						else {
							rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, false, false);
						}
					}
					else {
						if (currRule.isBuffered()) {
							if (currRule.getRuletype() == Ruletype::XRule and tail == *currRule.getHeadconstant()) {
								headresults_vec = currRule.getBuffer();
							}
							else if (currRule.getRuletype() == Ruletype::YRule and tail != *currRule.getHeadconstant()) {
								if (rulegraph->existsAcyclic(&tail, currRule, false)) {
									headresults_vec.push_back(*currRule.getHeadconstant());
								}
							}
						}
						else {
							throw new std::runtime_error("NOT BUFFERED");
						}
					}

					if (headresults_vec.size() > 0) {
						stop = true;
						for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
							if (fineScoreTrees[headIndex] == false) {
								int head = v_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
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
					int head = v_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
					// Get Headresults and final sorting
					std::vector<std::pair<int, double>> headresults_vec;
					headScoreTrees[headIndex].getResults(headresults_vec);
					std::sort(headresults_vec.begin(), headresults_vec.end(), finalResultComperator);
						

					int size_heads = headresults_vec.size() > k ? k : headresults_vec.size();
					int rank;
					for (int i = size_heads - 1; i >= 0; i--) {
						if (i == size_heads - 1 or headresults_vec[i].second != headresults_vec[i + 1].second) {
							rank = i;
						}
						if (head == headresults_vec[i].first) {
							mrr_head = mrr_head + (1.0 / (rank + 1));
						}
					}
					predicted_head++;
					headScoreTrees[headIndex].Free();
				}
				delete[] headScoreTrees;
			}
			tails++;
		}
	}

	return std::make_pair(((double)mrr_head / (double)predicted_head), ((double)mrr_tail / (double)predicted_tail));
}

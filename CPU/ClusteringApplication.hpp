#ifndef CLUSTERINGAPPL_H
#define CLUSTERINGAPPL_H

#include "RuleEngine.hpp"
#include "Util.hpp"
#include <map>
#include <functional>

class ClusteringApplication : public RuleEngine
{
public:

	ClusteringApplication(std::unordered_map<int, std::pair<double, std::vector<std::unordered_set<int>>>> rel2clusters, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) : RuleEngine(index, graph, ttr, vtr, rr) {
		this->rel2clusters = rel2clusters;
		it = ttr->getUniqueRelations().begin();
	}

	void start() {
		std::thread* threads = new std::thread[WORKER_THREADS];
		out = new std::stringstream[WORKER_THREADS];

		fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");


		//Foreach testtriples
		for (int i = 0; i < WORKER_THREADS; i++) {
			threads[i] = std::thread(&ClusteringApplication::run, this, i);
		}

		for (int i = 0; i < WORKER_THREADS; i++) {
			threads[i].join();
		}

		fclose(pFile);
	}

	void run(int threadId) {
		while (true) {
			int rel = getNextRel();
			if (rel == -1) {
				break;
			}
			std::cout << rel << "\n";
			std::pair<double, std::vector<std::unordered_set<int>>> cluster = rel2clusters[rel];
			if (cluster.first == 0) {
				max(rel);
			}
			else {
				noisy(rel, cluster.second);
			}
		}
	}


	void noisy(int rel, std::vector<std::unordered_set<int>> clusters) {

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

		std::unordered_map<int, std::unordered_set<int>>* relCounter = graph->getRelCounter();

		std::unordered_map<int, std::unordered_map<int, std::map<int, double>>> headTailResults;
		std::unordered_map<int, std::unordered_map<int, std::map<int, double>>> tailHeadResults;

		int ind_ptr = adj_begin[3 + rel];
		int lenRules = adj_begin[3 + rel + 1] - ind_ptr;

		{
			// adj list of testtriple x r ?
			int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2]]);
			int lenHeads = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

			auto heads = ttr->getRelHeadToTails()[rel].begin();
			while (heads != ttr->getRelHeadToTails()[rel].end()) {
				for (int i = 0; i < clusters.size(); i++) {
					std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, double>>> headTailResults_cluster;

					int head = heads->first;
					int* head_ind_ptr = &t_adj_list[3 + head];

					int lenTails = heads->second.size();

					if (lenTails > 0) {

						int ruleIndex = 0;
						for (auto ruleIndex : clusters[i]) {
							Rule currRule = rules_adj_list[ind_ptr + ruleIndex];
							std::vector<int> tailresults_vec;

							if (currRule.getRuletype() == Ruletype::XRule) {
								if (existsAcyclic(adj_list_starts, adj_lists, &head, currRule)) {
									tailresults_vec.push_back(*(currRule.getHeadconstant()));
								}
							}
							else if (currRule.getRuletype() == Ruletype::YRule) {
								if (*currRule.getHeadconstant() == head) {
									computeAcyclic(adj_list_starts, adj_lists, currRule, tailresults_vec);
								}
							}
							else {
								computeTailsCyclic(adj_list_starts, adj_lists, &head, currRule, tailresults_vec);
							}


							if (tailresults_vec.size() > 0) {
								int* tailresults = new int[tailresults_vec.size()];
								std::copy(tailresults_vec.begin(), tailresults_vec.end(), tailresults);
								std::sort(tailresults, (tailresults + tailresults_vec.size()));
								int* end = std::unique(tailresults, (tailresults + tailresults_vec.size()));

								//Filter results from trainingsset
								int* adj_list = &(adj_lists[adj_list_starts[rel * 2]]);
								int* indptr = &adj_list[3 + head];
								int len = *(indptr + 1) - *indptr;
								int* ind = &adj_list[3 + adj_list[1] + *indptr];
								int* end_diff_train = std::set_difference(tailresults, end, ind, ind + len, tailresults);

								//Filter results from valset
								int* v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2]]);
								int* v_indptr = &v_adj_list[3 + head];
								int v_len = *(v_indptr + 1) - *v_indptr;
								int* v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
								int* end_diff_val = std::set_difference(tailresults, end_diff_train, v_ind, v_ind + v_len, tailresults);


								for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {

									int* tail = &t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
									//Filter results from testset
									int* t_indptr = &t_adj_list[3 + head];
									int t_len = *(t_indptr + 1) - *t_indptr;
									int* t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
									int* tailresults_testsetfiltered = new int[tailresults_vec.size()];
									int* end_diff_test = util::test_set_difference(tailresults, end_diff_val, t_ind, t_ind + t_len, tailresults_testsetfiltered, *tail);
									int nValues = std::distance(tailresults_testsetfiltered, end_diff_test);


									std::vector<int> tailresults_vec(tailresults_testsetfiltered, tailresults_testsetfiltered + nValues);

									for (auto tailresult : tailresults_vec) {

										if (currRule.is_c()) {
											if (currRule.tail_exceptions.find(tailresult) != currRule.tail_exceptions.end()) {
												continue;
											}
										}

										if (Properties::get().ONLY_UNCONNECTED == 1) {
											if ((*relCounter).find(head) != (*relCounter).end()) {
												auto& it = (*relCounter).find(head)->second;
												if (it.find(tailresult) != it.end()) {
													continue;
												}
											}
										}

										auto it = headTailResults_cluster[head][*tail].find(tailresult);
										if (it == headTailResults_cluster[head][*tail].end()) {
											headTailResults_cluster[head][*tail][tailresult] = currRule.getAppliedConfidence();
										}
										else {
											if (headTailResults_cluster[head][*tail][tailresult] < currRule.getAppliedConfidence()) {
												headTailResults_cluster[head][*tail][tailresult] = currRule.getAppliedConfidence();
											}
										}
									}
									delete[] tailresults_testsetfiltered;
								}
								delete[] tailresults;
							}
						}
					}
					auto it_head = headTailResults_cluster.begin();
					while (it_head != headTailResults_cluster.end()) {
						auto it_tail = it_head->second.begin();
						while (it_tail != it_head->second.end()) {
							for (std::pair<int, double> res : it_tail->second) {
								auto it = headTailResults[it_head->first][it_tail->first].find(res.first);
								if (it == headTailResults[it_head->first][it_tail->first].end()) {
									headTailResults[it_head->first][it_tail->first][res.first] = 1.0;
								}
								headTailResults[it_head->first][it_tail->first][res.first] *= (1.0 - res.second);
							}
							it_tail++;
						}
						it_head++;
					}
				}
				heads++;
			}
		}

		{
			// adj list of testtriple ? r y
			int* t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2 + 1]]);
			int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

			auto tails = ttr->getRelTailToHeads()[rel].begin();
			while (tails != ttr->getRelTailToHeads()[rel].end()) {
				for (int i = 0; i < clusters.size(); i++) {
					std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, double>>> tailHeadResults_cluster;

					int tail = tails->first;
					int* tail_ind_ptr = &t_adj_list[3 + tail];
					int lenHeads = *(tail_ind_ptr + 1) - *tail_ind_ptr;

					if (lenHeads > 0) {
						int ruleIndex = 0;
						for (auto ruleIndex : clusters[i]) {
							Rule currRule = rules_adj_list[ind_ptr + ruleIndex];
							std::vector<int> headresults_vec;

							if (currRule.getRuletype() == Ruletype::YRule) {
								if (existsAcyclic(adj_list_starts, adj_lists, &tail, currRule)) {
									headresults_vec.push_back(*(currRule.getHeadconstant()));
								}
							}
							else if (currRule.getRuletype() == Ruletype::XRule) {
								if (*currRule.getHeadconstant() == tail) {
									computeAcyclic(adj_list_starts, adj_lists, currRule, headresults_vec);
								}
							}
							else {
								computeHeadsCyclic(adj_list_starts, adj_lists, &tail, currRule, headresults_vec);
							}

							if (headresults_vec.size() > 0) {
								int* headresults = new int[headresults_vec.size()];
								std::copy(headresults_vec.begin(), headresults_vec.end(), headresults);
								// Calculate unique results
								std::sort(headresults, (headresults + headresults_vec.size()));
								int* end = std::unique(headresults, (headresults + headresults_vec.size()));

								//Filter results from trainingsset
								int* adj_list = &(adj_lists[adj_list_starts[rel * 2 + 1]]);
								int* indptr = &adj_list[3 + tail];
								int len = *(indptr + 1) - *indptr;
								int* ind = &adj_list[3 + adj_list[1] + *indptr];
								int* end_diff_train = std::set_difference(headresults, end, ind, ind + len, headresults);

								//Filter results from valset
								int* v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2 + 1]]);
								int* v_indptr = &v_adj_list[3 + tail];
								int v_len = *(v_indptr + 1) - *v_indptr;
								int* v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
								int* end_diff_val = std::set_difference(headresults, end_diff_train, v_ind, v_ind + v_len, headresults);

								for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
									int* head = &t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
									//Filter results from testset
									int* t_indptr = &t_adj_list[3 + tail];
									int t_len = *(t_indptr + 1) - *t_indptr;
									int* t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
									int* headresults_testsetfiltered = new int[headresults_vec.size()];
									int* end_diff_test = util::test_set_difference(headresults, end_diff_val, t_ind, t_ind + t_len, headresults_testsetfiltered, *head);

									int nValues = std::distance(headresults_testsetfiltered, end_diff_test);

									std::vector<int> headresults_vec(headresults_testsetfiltered, headresults_testsetfiltered + nValues);

									for (auto headresult : headresults_vec) {

										if (currRule.is_c()) {
											if (currRule.head_exceptions.find(headresult) != currRule.head_exceptions.end()) {
												continue;
											}
										}

										if (Properties::get().ONLY_UNCONNECTED == 1) {
											if ((*relCounter).find(tail) != (*relCounter).end()) {
												auto& it = (*relCounter).find(tail)->second;
												if (it.find(headresult) != it.end()) {
													continue;
												}
											}
										}

										auto it = tailHeadResults_cluster[tail][*head].find(headresult);
										if (it == tailHeadResults_cluster[tail][*head].end()) {
											tailHeadResults_cluster[tail][*head][headresult] = currRule.getAppliedConfidence();
										}
										else {
											if (tailHeadResults_cluster[tail][*head][headresult] < currRule.getAppliedConfidence()) {
												tailHeadResults_cluster[tail][*head][headresult] = currRule.getAppliedConfidence();
											}
										}
									}
									delete[] headresults_testsetfiltered;
								}
								delete[] headresults;
							}
						}
					}
					auto it_tail = tailHeadResults_cluster.begin();
					while (it_tail != tailHeadResults_cluster.end()) {
						auto it_head = it_tail->second.begin();
						while (it_head != it_tail->second.end()) {
							for (std::pair<int, double> res : it_head->second) {
								auto it = tailHeadResults[it_tail->first][it_head->first].find(res.first);
								if (it == tailHeadResults[it_tail->first][it_head->first].end()) {
									tailHeadResults[it_tail->first][it_head->first][res.first] = 1.0;
								}
								tailHeadResults[it_tail->first][it_head->first][res.first] *= (1.0 - res.second);
							}
							it_head++;
						}
						it_tail++;
					}
				}
				tails++;
			}
		}

		auto it_head = headTailResults.begin();
		while (it_head != headTailResults.end()) {
			auto it_tail = it_head->second.begin();
			while (it_tail != it_head->second.end()) {
				auto headTailResult = headTailResults[it_head->first][it_tail->first];
				auto tailHeadResult = tailHeadResults[it_tail->first][it_head->first];

				std::vector<std::pair<int, double>> headresVector;
				auto it_headstart = tailHeadResult.begin();
				while (it_headstart != tailHeadResult.end()) {
					headresVector.push_back(std::make_pair(it_headstart->first, 1.0 - it_headstart->second));
					it_headstart++;
				}
				std::sort(headresVector.begin(), headresVector.end(), compFunctor);

				std::vector<std::pair<int, double>> tailresVector;
				auto it_tailstart = headTailResult.begin();
				while (it_tailstart != headTailResult.end()) {
					tailresVector.push_back(std::make_pair(it_tailstart->first, 1.0 - it_tailstart->second));
					it_tailstart++;
				}
				std::sort(tailresVector.begin(), tailresVector.end(), compFunctor);



				writeTopKCandidates(
					it_head->first,
					rel,
					it_tail->first,
					headresVector,
					tailresVector,
					pFile,
					Properties::get().TOP_K_OUTPUT
				);
				it_tail++;
			}
			it_head++;
		}
	}

	void max(int relation) {
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

		std::unordered_map<int, std::unordered_set<int>>* relCounter = graph->getRelCounter();

		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> headTailResults;
		std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> tailHeadResults;

		int ind_ptr = adj_begin[3 + relation];
		int lenRules = adj_begin[3 + relation + 1] - ind_ptr;

		{
			// adj list of testtriple x r ?
			int* t_adj_list = &(tt_adj_list[tt_adj_begin[relation * 2]]);
			int lenHeads = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

			auto heads = ttr->getRelHeadToTails()[relation].begin();
			while (heads != ttr->getRelHeadToTails()[relation].end()) {
				int head = heads->first;
				int* head_ind_ptr = &t_adj_list[3 + head];

				int lenTails = heads->second.size();

				if (lenTails > 0) {
					ScoreTree* tailScoreTrees = new ScoreTree[lenTails];
					bool* fineScoreTrees = new bool[lenTails];
					for (int z = 0; z < lenTails; z++) { fineScoreTrees[z] = false; }
					int ruleIndex = 0;
					bool stop = false;

					while (stop == false && ruleIndex < lenRules) {
						Rule currRule = rules_adj_list[ind_ptr + ruleIndex];
						std::vector<int> tailresults_vec;

						if (currRule.getRuletype() == Ruletype::XRule) {
							if (existsAcyclic(adj_list_starts, adj_lists, &head, currRule)) {
								tailresults_vec.push_back(*(currRule.getHeadconstant()));
							}
						}
						else if (currRule.getRuletype() == Ruletype::YRule) {
							if (*currRule.getHeadconstant() == head) {
								computeAcyclic(adj_list_starts, adj_lists, currRule, tailresults_vec);
							}
						}
						else {
							computeTailsCyclic(adj_list_starts, adj_lists, &head, currRule, tailresults_vec);
						}

						if (tailresults_vec.size() > 0) {

							std::vector<int> tailresults_vec_filtered;
							for (auto tailresult : tailresults_vec) {
								if (currRule.is_c()) {
									if (currRule.tail_exceptions.find(tailresult) != currRule.tail_exceptions.end()) {
										continue;
									}
								}

								if (Properties::get().ONLY_UNCONNECTED == 1) {
									if ((*relCounter).find(head) != (*relCounter).end()) {
										auto& it = (*relCounter).find(head)->second;
										if (it.find(tailresult) != it.end()) {
											continue;
										}
									}
								}
								tailresults_vec_filtered.push_back(tailresult);
							}

							int* tailresults = new int[tailresults_vec_filtered.size()];
							std::copy(tailresults_vec_filtered.begin(), tailresults_vec_filtered.end(), tailresults);
							std::sort(tailresults, (tailresults + tailresults_vec_filtered.size()));
							int* end = std::unique(tailresults, (tailresults + tailresults_vec_filtered.size()));

							//Filter results from trainingsset
							int* adj_list = &(adj_lists[adj_list_starts[relation * 2]]);
							int* indptr = &adj_list[3 + head];
							int len = *(indptr + 1) - *indptr;
							int* ind = &adj_list[3 + adj_list[1] + *indptr];
							int* end_diff_train = std::set_difference(tailresults, end, ind, ind + len, tailresults);

							//Filter results from valset
							int* v_adj_list = &(vt_adj_list[vt_adj_begin[relation * 2]]);
							int* v_indptr = &v_adj_list[3 + head];
							int v_len = *(v_indptr + 1) - *v_indptr;
							int* v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
							int* end_diff_val = std::set_difference(tailresults, end_diff_train, v_ind, v_ind + v_len, tailresults);


							stop = true;
							for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
								if (fineScoreTrees[tailIndex] == false) {
									int* tail = &t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
									//Filter results from testset
									int* t_indptr = &t_adj_list[3 + head];
									int t_len = *(t_indptr + 1) - *t_indptr;
									int* t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
									int* tailresults_testsetfiltered = new int[tailresults_vec_filtered.size()];;

									int* end_diff_test = util::test_set_difference(tailresults, end_diff_val, t_ind, t_ind + t_len, tailresults_testsetfiltered, *tail);

									int nValues = std::distance(tailresults_testsetfiltered, end_diff_test);
									tailScoreTrees[tailIndex].addValues(currRule.getAppliedConfidence(), tailresults_testsetfiltered, nValues);
									delete[] tailresults_testsetfiltered;

									if (tailScoreTrees[tailIndex].fine()) {
										fineScoreTrees[tailIndex] = true;
									}
									else {
										stop = false;
									}
								}
							}
							delete[] tailresults;
						}
						ruleIndex++;
					}
					for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
						int* tail = &t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
						auto cmp = [](std::pair<int, double> const& a, std::pair<int, double> const& b)
						{
							return a.second > b.second;
						};

						// Get Tailresults and final sorting
						std::vector<std::pair<int, double>> tailresults_vec;
						tailScoreTrees[tailIndex].getResults(tailresults_vec);
						std::sort(tailresults_vec.begin(), tailresults_vec.end(), finalResultComperator);
						headTailResults[head][*tail] = tailresults_vec;
						tailScoreTrees[tailIndex].Free();
					}
					delete[] fineScoreTrees;
					delete[] tailScoreTrees;
				}
				heads++;
			}
		}

		{
			// adj list of testtriple ? r y
			int* t_adj_list = &(tt_adj_list[tt_adj_begin[relation * 2 + 1]]);
			int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

			auto tails = ttr->getRelTailToHeads()[relation].begin();
			while (tails != ttr->getRelTailToHeads()[relation].end()) {
				int tail = tails->first;
				int* tail_ind_ptr = &t_adj_list[3 + tail];

				int lenHeads = *(tail_ind_ptr + 1) - *tail_ind_ptr;

				if (lenHeads > 0) {
					ScoreTree* headScoreTrees = new ScoreTree[lenHeads];
					bool* fineScoreTrees = new bool[lenHeads];
					for (int z = 0; z < lenHeads; z++) { fineScoreTrees[z] = false; }
					bool stop = false;
					int ruleIndex = 0;
					while (stop == false && ruleIndex < lenRules) {
						Rule currRule = rules_adj_list[ind_ptr + ruleIndex];
						std::vector<int> headresults_vec;

						if (currRule.getRuletype() == Ruletype::YRule) {
							if (existsAcyclic(adj_list_starts, adj_lists, &tail, currRule)) {
								headresults_vec.push_back(*(currRule.getHeadconstant()));
							}
						}
						else if (currRule.getRuletype() == Ruletype::XRule) {
							if (*currRule.getHeadconstant() == tail) {
								computeAcyclic(adj_list_starts, adj_lists, currRule, headresults_vec);
							}
						}
						else {
							computeHeadsCyclic(adj_list_starts, adj_lists, &tail, currRule, headresults_vec);
						}

						if (headresults_vec.size() > 0) {

							std::vector<int> headresults_vec_filtered;
							for (auto headresult : headresults_vec) {
								if (currRule.is_c()) {
									if (currRule.head_exceptions.find(headresult) != currRule.head_exceptions.end()) {
										continue;
									}
								}

								if (Properties::get().ONLY_UNCONNECTED == 1) {
									if ((*relCounter).find(tail) != (*relCounter).end()) {
										auto& it = (*relCounter).find(tail)->second;
										if (it.find(headresult) != it.end()) {
											continue;
										}
									}
								}
								headresults_vec_filtered.push_back(headresult);
							}

							int* headresults = new int[headresults_vec_filtered.size()];
							std::copy(headresults_vec_filtered.begin(), headresults_vec_filtered.end(), headresults);
							// Calculate unique results
							std::sort(headresults, (headresults + headresults_vec_filtered.size()));
							int* end = std::unique(headresults, (headresults + headresults_vec_filtered.size()));

							//Filter results from trainingsset
							int* adj_list = &(adj_lists[adj_list_starts[relation * 2 + 1]]);
							int* indptr = &adj_list[3 + tail];
							int len = *(indptr + 1) - *indptr;
							int* ind = &adj_list[3 + adj_list[1] + *indptr];
							int* end_diff_train = std::set_difference(headresults, end, ind, ind + len, headresults);

							//Filter results from valset
							int* v_adj_list = &(vt_adj_list[vt_adj_begin[relation * 2 + 1]]);
							int* v_indptr = &v_adj_list[3 + tail];
							int v_len = *(v_indptr + 1) - *v_indptr;
							int* v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
							int* end_diff_val = std::set_difference(headresults, end_diff_train, v_ind, v_ind + v_len, headresults);

							stop = true;
							for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
								if (fineScoreTrees[headIndex] == false) {
									int* head = &t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
									//Filter results from testset
									int* t_indptr = &t_adj_list[3 + tail];
									int t_len = *(t_indptr + 1) - *t_indptr;
									int* t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
									int* headresults_testsetfiltered = new int[headresults_vec_filtered.size()];
									int* end_diff_test = util::test_set_difference(headresults, end_diff_val, t_ind, t_ind + t_len, headresults_testsetfiltered, *head);

									int nValues = std::distance(headresults_testsetfiltered, end_diff_test);
									headScoreTrees[headIndex].addValues(currRule.getAppliedConfidence(), headresults_testsetfiltered, nValues);
									delete[] headresults_testsetfiltered;

									if (headScoreTrees[headIndex].fine()) {
										fineScoreTrees[headIndex] = true;
									}
									else {
										stop = false;
									}
								}
							}
							delete[] headresults;
						}
						ruleIndex++;
					}
					for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
						int* head = &t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
						// Get Headresults and final sorting
						std::vector<std::pair<int, double>> headresults_vec;
						headScoreTrees[headIndex].getResults(headresults_vec);
						std::sort(headresults_vec.begin(), headresults_vec.end(), finalResultComperator);
						tailHeadResults[tail][*head] = headresults_vec;
						headScoreTrees[headIndex].Free();
					}
					delete[] fineScoreTrees;
					delete[] headScoreTrees;
				}
				tails++;
			}
		}

		auto it_head = headTailResults.begin();
		while (it_head != headTailResults.end()) {
			auto it_tail = it_head->second.begin();
			while (it_tail != it_head->second.end()) {
				writeTopKCandidates(it_head->first, relation, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, Properties::get().TOP_K_OUTPUT);
				it_tail++;
			}
			it_head++;
		}
	}

private:

	std::unordered_map<int, std::pair<double, std::vector<std::unordered_set<int>>>> rel2clusters;

	typedef std::function<bool(std::pair<int, double>, std::pair<int, double>)> Comparator;
	Comparator compFunctor =
		[](std::pair<int, double> elem1, std::pair<int, double> elem2)
	{
		return elem1.second > elem2.second;
	};

	std::vector<int>::iterator it;
	int getNextRel() {
		int rel;
		lock->lock();
		if (it == ttr->getUniqueRelations().end()) {
			rel = -1;
		}
		else {
			rel = *it;
			it++;
		}
		lock->unlock();
		return rel;
	}
};


#endif

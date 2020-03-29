#ifndef FASTRULEENGINE_H
#define FASTRULEENGINE_H

#include "RuleEngine.hpp"

class FastRuleEngine : public RuleEngine
{
public:
	FastRuleEngine(Index * index, Graph * graph, TesttripleReader * ttr, ValidationtripleReader * vtr, RuleReader * rr) : RuleEngine(index, graph, ttr, vtr, rr){
		it = ttr->getUniqueRelations().begin();
	}

	void start() {
		std::thread * threads = new std::thread[WORKER_THREADS];
		out = new std::stringstream[WORKER_THREADS];

		fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");


		//Foreach testtriples
		for (int i = 0; i < WORKER_THREADS; i++) {
			threads[i] = std::thread(&FastRuleEngine::run, this, i);
		}

		for (int i = 0; i < WORKER_THREADS; i++) {
			threads[i].join();
		}

		fclose(pFile);
	}

	void run(int threadId) {
		int * adj_lists = graph->getCSR()->getAdjList();
		int * adj_list_starts = graph->getCSR()->getAdjBegin();

		Rule * rules_adj_list = rr->getCSR()->getAdjList();
		int * adj_begin = rr->getCSR()->getAdjBegin();

		int ** testtriples = ttr->getTesttriples();
		int * testtriplessize = ttr->getTesttriplesSize();

		int * tt_adj_list = ttr->getCSR()->getAdjList();
		int * tt_adj_begin = ttr->getCSR()->getAdjBegin();

		int * vt_adj_list = vtr->getCSR()->getAdjList();
		int * vt_adj_begin = vtr->getCSR()->getAdjBegin();

		//Foreach testtriples
		while (true) {

			int rel = getNextRel();
			if (rel == -1) { break; }

			std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> headTailResults;
			std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, double>>>> tailHeadResults;

			int ind_ptr = adj_begin[3 + rel];
			int lenRules = adj_begin[3 + rel + 1] - ind_ptr;

			{
				// adj list of testtriple x r ?
				int * t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2]]);
				int lenHeads = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

				int * tailresults = new int[DISCRIMINATION_BOUND];
				int * tailresultlength = new int(0);

				auto heads = ttr->getRelHeadToTails()[rel].begin();
				while(heads != ttr->getRelHeadToTails()[rel].end()) {
					int head = heads->first;
					int * head_ind_ptr = &t_adj_list[3 + head];

					int lenTails = heads->second.size();
					
					if (lenTails > 0) {
						ScoreTree * tailScoreTrees = new ScoreTree[lenTails];
						bool * fineScoreTrees = new bool[lenTails];
						for (int z = 0; z < lenTails; z++) { fineScoreTrees[z] = false; }
						int ruleIndex = 0;
						bool stop = false;

						while (stop == false && ruleIndex < lenRules) {
							Rule currRule = rules_adj_list[ind_ptr + ruleIndex];
							if (currRule.getRuletype() == Ruletype::XRule) {
								if (existsAcyclic(adj_list_starts, adj_lists, &head, currRule)) {
									tailresults[*tailresultlength] = *(currRule.getHeadconstant());
									(*tailresultlength)++;
								}
							}
							else if (currRule.getRuletype() == Ruletype::YRule) {
								if (*currRule.getHeadconstant() == head) {
									computeAcyclic(adj_list_starts, adj_lists, currRule, tailresults, tailresultlength);
								}
							}
							else {
								computeTailsCyclic(adj_list_starts, adj_lists, &head, currRule, tailresults, tailresultlength);
							}

							if (*tailresultlength > 0) {
								std::sort(tailresults, (tailresults + *tailresultlength));
								int * end = std::unique(tailresults, (tailresults + *tailresultlength));

								//Filter results from trainingsset
								int * adj_list = &(adj_lists[adj_list_starts[rel * 2]]);
								int * indptr = &adj_list[3 + head];
								int len = *(indptr + 1) - *indptr;
								int * ind = &adj_list[3 + adj_list[1] + *indptr];
								int * end_diff_train = std::set_difference(tailresults, end, ind, ind + len, tailresults);

								//Filter results from valset
								int * v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2]]);
								int * v_indptr = &v_adj_list[3 + head];
								int v_len = *(v_indptr + 1) - *v_indptr;
								int * v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
								int * end_diff_val = std::set_difference(tailresults, end_diff_train, v_ind, v_ind + v_len, tailresults);

								stop = true;
								for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
									if (fineScoreTrees[tailIndex] == false) {
										int * tail = &t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
										//Filter results from testset
										int * t_indptr = &t_adj_list[3 + head];
										int t_len = *(t_indptr + 1) - *t_indptr;
										int * t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
										int * tailresults_testsetfiltered = new int[DISCRIMINATION_BOUND];;

										int * end_diff_test = std::set_difference(tailresults, end_diff_val, t_ind, t_ind + t_len, tailresults_testsetfiltered, testComp{ *tail });

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
								*tailresultlength = 0;
							}
							ruleIndex++;
						}
						for (int tailIndex = 0; tailIndex < lenTails; tailIndex++) {
							int * tail = &t_adj_list[3 + lenHeads + *head_ind_ptr + tailIndex];
							auto cmp = [](std::pair<int, double> const & a, std::pair<int, double> const & b)
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
				delete[] tailresults;
				delete tailresultlength;
			}

			{
				// adj list of testtriple ? r y
				int * t_adj_list = &(tt_adj_list[tt_adj_begin[rel * 2 + 1]]);
				int lenTails = t_adj_list[1]; // size + 1 of testtriple testtriple heads of a specific relation

				int * headresults = new int[DISCRIMINATION_BOUND];
				int * headresultlength = new int(0);

				auto tails = ttr->getRelTailToHeads()[rel].begin();
				while(tails != ttr->getRelTailToHeads()[rel].end()) {
					int tail = tails->first;
					int * tail_ind_ptr = &t_adj_list[3 + tail];

					int lenHeads = *(tail_ind_ptr + 1) - *tail_ind_ptr;

					if (lenHeads > 0) {
						ScoreTree * headScoreTrees = new ScoreTree[lenHeads];
						bool * fineScoreTrees = new bool[lenHeads];
						for (int z = 0; z < lenHeads; z++) { fineScoreTrees[z] = false; }
						bool stop = false;
						int ruleIndex = 0;
						while (stop == false && ruleIndex < lenRules) {
							Rule currRule = rules_adj_list[ind_ptr + ruleIndex];
							if (currRule.getRuletype() == Ruletype::YRule) {
								if (existsAcyclic(adj_list_starts, adj_lists, &tail, currRule)) {
									headresults[*headresultlength] = *(currRule.getHeadconstant());
									(*headresultlength)++;
								}
							}
							else if (currRule.getRuletype() == Ruletype::XRule) {
								if (*currRule.getHeadconstant() == tail) {
									computeAcyclic(adj_list_starts, adj_lists, currRule, headresults, headresultlength);
								}
							}
							else {
								computeHeadsCyclic(adj_list_starts, adj_lists, &tail, currRule, headresults, headresultlength);
							}

							if (*headresultlength > 0) {
								// Calculate unique results
								std::sort(headresults, (headresults + *headresultlength));
								int * end = std::unique(headresults, (headresults + *headresultlength));

								//Filter results from trainingsset
								int * adj_list = &(adj_lists[adj_list_starts[rel * 2 + 1]]);
								int * indptr = &adj_list[3 + tail];
								int len = *(indptr + 1) - *indptr;
								int * ind = &adj_list[3 + adj_list[1] + *indptr];
								int * end_diff_train = std::set_difference(headresults, end, ind, ind + len, headresults);

								//Filter results from valset
								int * v_adj_list = &(vt_adj_list[vt_adj_begin[rel * 2 + 1]]);
								int * v_indptr = &v_adj_list[3 + tail];
								int v_len = *(v_indptr + 1) - *v_indptr;
								int * v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
								int * end_diff_val = std::set_difference(headresults, end_diff_train, v_ind, v_ind + v_len, headresults);

								stop = true;
								for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
									if (fineScoreTrees[headIndex] == false) {
										int * head = &t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
										//Filter results from testset
										int * t_indptr = &t_adj_list[3 + tail];
										int t_len = *(t_indptr + 1) - *t_indptr;
										int * t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
										int * headresults_testsetfiltered = new int[DISCRIMINATION_BOUND];
										int * end_diff_test = std::set_difference(headresults, end_diff_val, t_ind, t_ind + t_len, headresults_testsetfiltered, testComp{ *head });

										int nValues = std::distance(headresults_testsetfiltered, end_diff_test);
										headScoreTrees[headIndex].addValues(currRule.getAppliedConfidence(), headresults_testsetfiltered, nValues);
										delete[] headresults_testsetfiltered;

										if (headScoreTrees[headIndex].fine()) {
											fineScoreTrees[headIndex] = true;
										} else {
											stop = false; 
										}
									}
								}
								*headresultlength = 0;
							}
							ruleIndex++;
						}
						for (int headIndex = 0; headIndex < lenHeads; headIndex++) {
							int * head = &t_adj_list[3 + lenTails + *tail_ind_ptr + headIndex];
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
				delete[] headresults;
				delete headresultlength;
			}

			auto it_head = headTailResults.begin();
			while (it_head != headTailResults.end()) {
				auto it_tail = it_head->second.begin();
				while (it_tail != it_head->second.end()) {
					writeTopKCandidates(it_head->first, rel, it_tail->first, tailHeadResults[it_tail->first][it_head->first], it_tail->second, pFile, Properties::get().TOP_K_OUTPUT);
					it_tail++;
				}
				it_head++;
			}

			// Write Results to stringstream
			//writeTopKCandiates(testtriple, headresults_vec, tailresults_vec, out[threadId], Properties::get().TOP_K_OUTPUT);
		}
	}

	private:

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


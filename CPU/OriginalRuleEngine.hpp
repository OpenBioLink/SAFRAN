#ifndef ORIGINALRULEENGINE_H
#define ORIGINALRULEENGINE_H

#include "RuleEngine.hpp"

#include <thread>
#include "Util.hpp"

class OriginalRuleEngine : public RuleEngine
{
public:
	OriginalRuleEngine(Index * index, Graph * graph, TesttripleReader * ttr, ValidationtripleReader * vtr, RuleReader * rr) : RuleEngine(index, graph, ttr, vtr, rr){}

	void start() {
		std::thread * threads = new std::thread[WORKER_THREADS];
		out = new std::stringstream[WORKER_THREADS];

		fopen_s(&pFile, Properties::get().PATH_OUTPUT.c_str(), "w");

		//Foreach testtriples
		for (int i = 0; i < WORKER_THREADS; i++) {
			threads[i] = std::thread(&OriginalRuleEngine::run, this, i);
		}

		for (int i = 0; i < WORKER_THREADS; i++) {
			threads[i].join();
		}

		fclose(pFile);
	}

private:
	int testtripleIndex = 0;

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
			int i = getNextTesttripleIndex();
			if (i >= *testtriplessize) break;

			int * testtriple = testtriples[i];

			int ind_ptr = adj_begin[3 + testtriple[1]];
			int len = adj_begin[3 + testtriple[1] + 1] - ind_ptr;

			//Calculate Tail Results
			int * tailresults = new int[DISCRIMINATION_BOUND];
			int * tailresultlength = new int(0);
			ScoreTree tailScoreTree = ScoreTree();

			//Foreach rule
			for (int j = 0; j < len; j++) {
				if (tailScoreTree.fine()) { break; }
				Rule currRule = rules_adj_list[ind_ptr + j];
				if (currRule.getRuletype() == Ruletype::XRule) {
					if (existsAcyclic(adj_list_starts, adj_lists, &testtriple[0], currRule)) {
						tailresults[*tailresultlength] = *(currRule.getHeadconstant());
						(*tailresultlength)++;
					}
				}
				else if (currRule.getRuletype() == Ruletype::YRule) {
					if (*currRule.getHeadconstant() == testtriple[0]) {
						computeAcyclic(adj_list_starts, adj_lists, currRule, tailresults, tailresultlength);
					}
				}
				else {
					computeTailsCyclic(adj_list_starts, adj_lists, &testtriple[0], currRule, tailresults, tailresultlength);
				}

				if (*tailresultlength > 0) {
					std::sort(tailresults, (tailresults + *tailresultlength));
					int * end = std::unique(tailresults, (tailresults + *tailresultlength));

					//Filter results from trainingsset
					int * adj_list = &(adj_lists[adj_list_starts[testtriple[1] * 2]]);
					int * indptr = &adj_list[3 + testtriple[0]];
					int len = *(indptr + 1) - *indptr;
					int * ind = &adj_list[3 + adj_list[1] + *indptr];
					auto endDiff = std::set_difference(tailresults, end, ind, ind + len, tailresults);

					//Filter results from valset
					int * v_adj_list = &(vt_adj_list[vt_adj_begin[testtriple[1] * 2]]);
					int * v_indptr = &v_adj_list[3 + testtriple[0]];
					int v_len = *(v_indptr + 1) - *v_indptr;
					int * v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
					endDiff = std::set_difference(tailresults, endDiff, v_ind, v_ind + v_len, tailresults);

					int * t_adj_list = &(tt_adj_list[tt_adj_begin[testtriple[1] * 2]]);
					int * t_indptr = &t_adj_list[3 + testtriple[0]];
					int t_len = *(t_indptr + 1) - *t_indptr;
					int * t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
					endDiff = util::test_set_difference(tailresults, endDiff, t_ind, t_ind + t_len, tailresults, testtriple[2]);

					//Add tailresults to ScoreTree
					int nValues = std::distance(tailresults, endDiff);
					tailScoreTree.addValues(currRule.getAppliedConfidence(), tailresults, nValues);
					*tailresultlength = 0;
				}
			}

			//Calculate Head Results
			int * headresults = new int[DISCRIMINATION_BOUND];
			int * headresultlength = new int(0);
			ScoreTree headScoreTree = ScoreTree();

			//Foreach rule
			for (int j = 0; j < len; j++) {
				if (headScoreTree.fine()) { break; }
				Rule currRule = rules_adj_list[ind_ptr + j];

				if (currRule.getRuletype() == Ruletype::YRule) {
					if (existsAcyclic(adj_list_starts, adj_lists, &testtriple[2], currRule)) {
						headresults[*headresultlength] = *(currRule.getHeadconstant());
						(*headresultlength)++;
					}
				}
				else if (currRule.getRuletype() == Ruletype::XRule) {
					if (*currRule.getHeadconstant() == testtriple[2]) {
						computeAcyclic(adj_list_starts, adj_lists, currRule, headresults, headresultlength);
					}
				}
				else {
					computeHeadsCyclic(adj_list_starts, adj_lists, &testtriple[2], currRule, headresults, headresultlength);
				}

				if (*headresultlength > 0) {
					// Calculate unique results
					std::sort(headresults, (headresults + *headresultlength));
					int * end = std::unique(headresults, (headresults + *headresultlength));

					//Filter results from trainingsset
					int * adj_list = &(adj_lists[adj_list_starts[testtriple[1] * 2 + 1]]);
					int * indptr = &adj_list[3 + testtriple[2]];
					int len = *(indptr + 1) - *indptr;
					int * ind = &adj_list[3 + adj_list[1] + *indptr];
					auto endDiff = std::set_difference(headresults, end, ind, ind + len, headresults);

					//Filter results from valset
					int * v_adj_list = &(vt_adj_list[vt_adj_begin[testtriple[1] * 2 + 1]]);
					int * v_indptr = &v_adj_list[3 + testtriple[2]];
					int v_len = *(v_indptr + 1) - *v_indptr;
					int * v_ind = &v_adj_list[3 + v_adj_list[1] + *v_indptr];
					endDiff = std::set_difference(headresults, endDiff, v_ind, v_ind + v_len, headresults);

					//Filter results from testset
					int * t_adj_list = &(tt_adj_list[tt_adj_begin[testtriple[1] * 2 + 1]]);
					int * t_indptr = &t_adj_list[3 + testtriple[2]];
					int t_len = *(t_indptr + 1) - *t_indptr;
					int * t_ind = &t_adj_list[3 + t_adj_list[1] + *t_indptr];
					endDiff = util::test_set_difference(headresults, endDiff, t_ind, t_ind + t_len, headresults, testtriple[0]);

					//Add headreults to ScoreTree
					int nValues = std::distance(headresults, endDiff);
					headScoreTree.addValues(currRule.getAppliedConfidence(), headresults, nValues);
					*headresultlength = 0;
				}
			}

			// Get Tailresults and final sorting
			std::vector<std::pair<int, double>> tailresults_vec;
			tailScoreTree.getResults(tailresults_vec);
			std::sort(tailresults_vec.begin(), tailresults_vec.end(), finalResultComperator);

			// Get Headresults and final sorting
			std::vector<std::pair<int, double>> headresults_vec;
			headScoreTree.getResults(headresults_vec);
			std::sort(headresults_vec.begin(), headresults_vec.end(), finalResultComperator);

			// Write Results to stringstream
			writeTopKCandidates(testtriple[0], testtriple[1], testtriple[2], headresults_vec, tailresults_vec, pFile, Properties::get().TOP_K_OUTPUT);

			delete[] headresults;
			delete headresultlength;
			delete[] tailresults;
			delete tailresultlength;
			headScoreTree.Free();
			tailScoreTree.Free();
		}
	}

	int getNextTesttripleIndex() {
		int ret;
		lock->lock();
		testtripleIndex++;
		ret = testtripleIndex;
		lock->unlock();
		return ret;
	}
};

#endif // ORIGINALRULEENGINE_H


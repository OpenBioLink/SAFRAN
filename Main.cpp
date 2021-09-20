#include "Index.h"
#include "TraintripleReader.h"
#include "RuleReader.h"
#include "TesttripleReader.h"
#include "ValidationtripleReader.h"
#include "Properties.hpp"
#include "ApplicationEngine.h"
#include "ClusteringEngine.h"
#include "ClusteringReader.h"
#include "RuleApplication.h"
#include "JaccardEngine.h"
#include <chrono>
#include <stdio.h>



int main(int argc, char** argv)
{

	
	if (argc != 3) {
		std::cout << "Wrong number of startup arguments, please make sure that arguments are in form of {action} {path to properties}" << std::endl;
		exit(-1);
	}
	Properties::get().ACTION = argv[1];
	bool success = Properties::get().read(argv[2]);
	if (!success) {
		std::cout << "No properties file found, falling back to default\n";
	}
	std::cout << Properties::get().toString() << "\n";

	if (Properties::get().WORKER_THREADS != -1) {
		omp_set_num_threads(Properties::get().WORKER_THREADS);
	}

	auto start = std::chrono::high_resolution_clock::now();
	Index* index = new Index();

	index->addNode(Properties::get().REFLEXIV_TOKEN);

	std::cout << "Reading trainingset..." << std::endl;
	//"C:\\Users\\Simon\\Desktop\\data\\train.txt"
	TraintripleReader* graph = new TraintripleReader(Properties::get().PATH_TRAINING, index);
	auto finish = std::chrono::high_resolution_clock::now();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Trainingset read and indexed in " << milliseconds.count() << " ms\n";
	start = finish;

	Properties::get().REL_SIZE = index->getRelSize();


	//"C:\\Users\\Simon\\Desktop\\data\\alpha-50"
	std::cout << "Reading rules..." << std::endl;
	RuleReader* rr = new RuleReader(Properties::get().PATH_RULES, index, graph);
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Rules read in " << milliseconds.count() << " ms\n";
	start = finish;

	std::cout << "Reading testset..." << std::endl;
	TesttripleReader* ttr = new TesttripleReader(Properties::get().PATH_TEST, index, graph, Properties::get().TRIAL);
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Testset read in " << milliseconds.count() << " ms\n";
	start = finish;

	std::cout << "Reading validationset..." << std::endl;
	ValidationtripleReader* vtr = new ValidationtripleReader(Properties::get().PATH_VALID, index, graph);
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Validationset read in " << milliseconds.count() << " ms\n";
	start = finish;

	/*
	Rule* rules_adj_list = rr->getCSR()->getAdjList();
	int* adj_begin = rr->getCSR()->getAdjBegin();
	std::string r("/award/award_winning_work/awards_won./award/award_honor/award_winner");
	int relation = *index->getIdOfRelationstring(r);

	std::string t("/m/0g69lg");
	int tail = *index->getIdOfNodestring(t);

	std::string h("/m/015ppk");
	int head = *index->getIdOfNodestring(h);

	auto tails = ttr->getRelTailToHeads()[relation][tail];

	int reflexiv_token = 0;

	int ind_ptr = adj_begin[3 + relation];
	int lenRules = adj_begin[3 + relation + 1] - ind_ptr;
	RuleGraph* rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
	for (int i = 0; i < lenRules; i++) {
		std::vector<int> headresults_vec;
		Rule& r = rules_adj_list[ind_ptr + i];
		if (r.getRulestring().compare("/award/award_winning_work/awards_won./award/award_honor/award_winner(/m/015ppk,Y) <= /award/award_winner/awards_won./award/award_honor/award_winner(Y,/m/05cqhl)") == 0) {
			if (r.is_ac2() && r.getRuletype() == Ruletype::YRule && *r.getHeadconstant() != tail) {
				if (rulegraph->existsAcyclic(&tail, r, true)) {
					headresults_vec.push_back(*r.getHeadconstant());
				}
			}
		}
		std::vector<int> filtered_headresults_vec;
		for (auto a : headresults_vec) {
			if (a == tail) continue;
			if (a == reflexiv_token) {
				a = tail;
			}
			if (a == head || tails.find(a) == tails.end()) {
				filtered_headresults_vec.push_back(a);
			}
		}
		for (auto a : filtered_headresults_vec) {
			std::cout << a << " " << *index->getStringOfNodeId(a) << " ";
		}
	}

	exit(-1);
	*/

	std::cout << "Applying rules..." << std::endl;

	if (Properties::get().ACTION.compare("learnnrnoisy") == 0) {
		ClusteringEngine* ce = new ClusteringEngine(index, graph, ttr, vtr, rr);
		ce->learn();
	}
	else if (Properties::get().ACTION.compare("applynrnoisy") == 0) {
		ClusteringReader* cr = new ClusteringReader(Properties::get().PATH_CLUSTER, rr->getCSR(), index, graph);
		RuleApplication* ca = new RuleApplication(index, graph, ttr, vtr, rr);
		ca->apply_nr_noisy(cr->getRelToClusters());
	}
	else if (Properties::get().ACTION.compare("applymax") == 0) {
		RuleApplication* ca = new RuleApplication(index, graph, ttr, vtr, rr);
		ca->apply_only_max();
	}
	else if (Properties::get().ACTION.compare("applynoisy") == 0) {
		RuleApplication* ca = new RuleApplication(index, graph, ttr, vtr, rr);
		ca->apply_only_noisy();
	}
	else if (Properties::get().ACTION.compare("calcjacc") == 0) {
		JaccardEngine* jacccalc = new JaccardEngine(index, graph, vtr, rr);
		jacccalc->calculate_jaccard();
	}
	else {
		std::cout << "ACTION not found" << "\n";
		exit(-1);
	}
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Rules read in " << milliseconds.count() << " ms\n";

}

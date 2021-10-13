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
#include "Explanation.h"
#include "Util.hpp"
#include <chrono>
#include <stdio.h>



int main(int argc, char** argv)
{

	
	if (argc != 3) {
		std::cout << "Wrong number of startup arguments, please make sure that arguments are in form of {action} {path to properties}" << std::endl;
		exit(-1);
	}
	Properties::get().setAction(argv[1]);
	Action action = Properties::get().ACTION;
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

	// Adding the reflexive token to the index 
	index->addNode(Properties::get().REFLEXIV_TOKEN);

	std::cout << "Reading trainingset..." << std::endl;
	//"C:\\Users\\Simon\\Desktop\\data\\train.txt"
	TraintripleReader* graph = new TraintripleReader(Properties::get().PATH_TRAINING, index);
	auto finish = std::chrono::high_resolution_clock::now();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Trainingset read and indexed in " << milliseconds.count() << " ms\n";
	start = finish;

	Properties::get().REL_SIZE = index->getRelSize();

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

	std::cout << "Reading rules..." << std::endl;
	RuleReader* rr = new RuleReader(Properties::get().PATH_RULES, index, graph);
	
	// READ Clustering if present
	ClusteringReader* cr = nullptr;
	if (action == applynrnoisy) {
		cr = new ClusteringReader(Properties::get().PATH_CLUSTER, rr->getCSR(), index, graph);
	}
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Rules read in " << milliseconds.count() << " ms\n";
	start = finish;

	

	// PREPARE Explanation DB if wanted
	Explanation* explanation = nullptr;
	if (Properties::get().EXPLAIN == 1) {
		std::cout << "Writing entities, relations and rules to db file..." << std::endl;
		explanation = new Explanation(Properties::get().PATH_EXPLAIN, true);
		explanation->begin_tr();
		explanation->insertEntities(index);
		explanation->insertRelations(index);
		explanation->insertRules(rr, index->getRelSize(), cr);
		finish = std::chrono::high_resolution_clock::now();
		milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
		std::cout << "Written in " << milliseconds.count() << " ms\n";
		start = finish;
	}

	std::cout << "Applying rules..." << std::endl;
	if (action == learnnrnoisy) {
		ClusteringEngine* ce = new ClusteringEngine(index, graph, ttr, vtr, rr);
		ce->learn();
	}
	else if (action == calcjacc) {
		JaccardEngine* jacccalc = new JaccardEngine(index, graph, vtr, rr);
		jacccalc->calculate_jaccard();
	}
	else {
		if (action == applynrnoisy) {
			RuleApplication* ca = new RuleApplication(index, graph, ttr, vtr, rr, explanation);
			ca->apply_nr_noisy(cr->getRelToClusters());
		}
		else if (action == applymax) {
			RuleApplication* ca = new RuleApplication(index, graph, ttr, vtr, rr, explanation);
			ca->apply_only_max();
		}
		else if (action == applynoisy) {
			RuleApplication* ca = new RuleApplication(index, graph, ttr, vtr, rr, explanation);
			ca->apply_only_noisy();
		}
		else {
			std::cout << "ACTION not found" << "\n";
			exit(-1);
		}
		if (Properties::get().EXPLAIN == 1) {
			explanation->commit_tr();
		}
	}
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Rules read in " << milliseconds.count() << " ms\n";

}

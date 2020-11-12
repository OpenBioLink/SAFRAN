#include "Index.hpp"
#include "TraintripleReader.hpp"
#include "RuleReader.hpp"
#include "TesttripleReader.hpp"
#include "ValidationtripleReader.hpp"
#include "Properties.hpp"
#include "NoisyOrEngine.hpp"
#include "ClusteringEngine.hpp"
#include "ClusteringReader.hpp"
#include "ClusteringApplication.hpp"
#include "JaccardCalculator.hpp"
#include <chrono>
#include <stdio.h>



int main(int argc, char** argv)
{
	/*
	if (argc != 2) {
		std::cout << "Wrong number of elements" << std::endl;
		exit(-1);
	}
	*/
	Properties::get().ACTION = argv[1];
	bool success = Properties::get().read(argv[2]);
	if (!success) {
		std::cout << "No properties file found, falling back to default\n";
	}
	std::cout << Properties::get().toString() << "\n";

	auto start = std::chrono::high_resolution_clock::now();
	Index* index = new Index();

	index->addNode(Properties::get().REFLEXIV_TOKEN);

	std::cout << "Reading trainingset..." << std::endl;
	//"C:\\Users\\Simon\\Desktop\\data\\train.txt"
	TraintripleReader * graph = new TraintripleReader(Properties::get().PATH_TRAINING, index);
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
	TesttripleReader* ttr = new TesttripleReader(Properties::get().PATH_TEST, index, graph);
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

	std::cout << "Applying rules..." << std::endl;

	if (Properties::get().ACTION.compare("learnnoisy") == 0) {
		ClusteringEngine* ce = new ClusteringEngine(index, graph, ttr, vtr, rr);
		ce->learn();
	}
	else if (Properties::get().ACTION.compare("applynoisy") == 0) {
		ClusteringReader* cr = new ClusteringReader(Properties::get().PATH_CLUSTER, rr->getCSR(), index, graph);
		ClusteringApplication* ca = new ClusteringApplication(cr->getRelToClusters(), index, graph, ttr, vtr, rr);
		ca->start();
	}
	else if (Properties::get().ACTION.compare("applymax") == 0) {

	}
	else if (Properties::get().ACTION.compare("calcjacc") == 0) {
		JaccardCalculator* jacccalc = new JaccardCalculator(index, graph, vtr, rr, 200);
		jacccalc->calculate_jaccard();
	}

	

	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	std::cout << "Rules read in " << milliseconds.count() << " ms\n";

}

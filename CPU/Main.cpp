#include "Index.hpp"
#include "Graph.hpp"
#include "RuleReader.hpp"
#include "TesttripleReader.hpp"
#include "ValidationtripleReader.hpp"
#include "OriginalRuleEngine.hpp"
#include "FastRuleEngine.hpp"
#include "Properties.hpp"

#include <chrono>
#include <stdio.h>


int main(int argc, char** argv)
{

	if (argc != 2) {
		std::cout << "Wrong number of elements" << std::endl;
		exit(-1);
	}
	Properties::get().read(argv[1]);

	auto start = std::chrono::high_resolution_clock::now();
	Index * index = new Index();

	//"C:\\Users\\Simon\\Desktop\\data\\train.txt"
	Graph * graph = new Graph(Properties::get().PATH_TRAINING, index);
	auto finish = std::chrono::high_resolution_clock::now();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
	std::cout << "Graph created in "<< milliseconds.count() << " ms\n";
	start = finish;


	//"C:\\Users\\Simon\\Desktop\\data\\alpha-50"
	RuleReader * rr = new RuleReader(Properties::get().PATH_RULES, index, graph);
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
	std::cout << "Rules created in "<< milliseconds.count() << " ms\n";
	start = finish;

	//"C:\\Users\\Simon\\Desktop\\data\\test.txt"
	TesttripleReader * ttr = new TesttripleReader(Properties::get().PATH_TEST, index, graph);
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
	std::cout << "Testtriples created in "<< milliseconds.count() << " ms\n";
	start = finish;

	ValidationtripleReader * vtr = new ValidationtripleReader(Properties::get().PATH_VALID, index, graph);
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
	std::cout << "Validationtriples created in "<< milliseconds.count() << " ms\n";
	start = finish;

	if (Properties::get().FAST == 0) {
		OriginalRuleEngine re = OriginalRuleEngine(index, graph, ttr, vtr, rr);
		re.start();

	}
	else {
		FastRuleEngine re = FastRuleEngine(index, graph, ttr, vtr, rr);
		re.start();
	}
	finish = std::chrono::high_resolution_clock::now();
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
	std::cout << "Rules applied in "<< milliseconds.count() << " ms\n";
	
	return 0;
}

#include "Index.hpp"
#include "Graph.hpp"
#include "RuleReader.hpp"
#include "TesttripleReader.hpp"
#include "ValidationtripleReader.hpp"
#include "OriginalRuleEngine.hpp"
#include "FastRuleEngine.hpp"
#include "Properties.hpp"

#include <ctime>
#include <stdio.h>


int main(int argc, char** argv)
{

	if (argc != 2) {
		std::cout << "Wrong number of elements" << std::endl;
		exit(-1);
	}
	Properties::get().read(argv[1]);

	unsigned int intermediate = clock();
	Index * index = new Index();

	//"C:\\Users\\Simon\\Desktop\\data\\train.txt"
	Graph * graph = new Graph(Properties::get().PATH_TRAINING, index);
	printf("Graph created in millisecs: %ld\n", clock() - intermediate);
	intermediate = clock();


	//"C:\\Users\\Simon\\Desktop\\data\\alpha-50"
	RuleReader * rr = new RuleReader(Properties::get().PATH_RULES, index, graph);
	printf("Rules created in millisecs: %ld\n", clock() - intermediate);
	intermediate = clock();

	//"C:\\Users\\Simon\\Desktop\\data\\test.txt"
	TesttripleReader * ttr = new TesttripleReader(Properties::get().PATH_TEST, index, graph);
	printf("Testtriples created in millisecs: %ld\n", clock() - intermediate);
	intermediate = clock();

	ValidationtripleReader * vtr = new ValidationtripleReader(Properties::get().PATH_VALID, index, graph);
	printf("Validationtriples created in millisecs: %ld\n", clock() - intermediate);
	intermediate = clock();

	if (Properties::get().FAST == 0) {
		OriginalRuleEngine re = OriginalRuleEngine(index, graph, ttr, vtr, rr);
		re.start();

	}
	else {
		FastRuleEngine re = FastRuleEngine(index, graph, ttr, vtr, rr);
		re.start();
	}
	std::cout << "Rules applied in millisecs: " << clock() - intermediate << std::endl;
	
	return 0;
}

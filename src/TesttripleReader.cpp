#include "TesttripleReader.h"

TesttripleReader::TesttripleReader(std::string filepath, Index * index, TraintripleReader* graph) {
	this->index = index;
	this->graph = graph;
	read(filepath);
}

int ** TesttripleReader::getTesttriples() {
	return testtriples;
}

int * TesttripleReader::getTesttriplesSize() {
	return testtripleSize;
}

CSR<int, int> * TesttripleReader::getCSR() {
	return csr;
}

std::vector<int>& TesttripleReader::getUniqueRelations() {
	return uniqueRelations;
}

RelNodeToNodes& TesttripleReader::getRelHeadToTails() {
	return relHeadToTails;
}

RelNodeToNodes& TesttripleReader::getRelTailToHeads() {
	return relTailToHeads;
}

void TesttripleReader::read(std::string filepath) {

	std::string line;
	std::ifstream myfile(filepath);
	if (myfile.is_open())
	{
		std::vector<std::vector<int*>> testtriplesVector;
		while (getline(myfile, line))
		{
			std::istringstream iss(line);
			std::vector<std::string> results = util::split(line, '\t');
			if (results.size() != 3) {
				std::cout << "Unsupported Filetype, please make sure you have the following triple format {subject}{TAB}{predicate}{TAB}{object}" << std::endl;
				exit(-1);
			}

			try {
				int* headId = index->getIdOfNodestring(results[0]);
				int* relId = index->getIdOfRelationstring(results[1]);
				int* tailId = index->getIdOfNodestring(results[2]);
				std::vector<int*> testtriple;
				testtriple.push_back(headId);
				testtriple.push_back(relId);
				testtriple.push_back(tailId);
				testtriplesVector.push_back(testtriple);

				relHeadToTails[*relId][*headId].insert(*tailId);
				relTailToHeads[*relId][*tailId].insert(*headId);
			}
			catch (std::runtime_error& e) {}
		}
		myfile.close();

		csr = new CSR<int, int>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);

		std::vector<int> keys(relHeadToTails.size());
		std::transform(relHeadToTails.begin(), relHeadToTails.end(), keys.begin(), [](auto pair) {return pair.first; });
		uniqueRelations = keys;

		if (Properties::get().TRIAL == 1) {
			if (Properties::get().FAST == 1) {
				std::cout << "Sorry, currently only the original rule engine is supported wir trials, please set FAST to 0." << std::endl;
			}
			Trial t = Trial(Properties::get().CONFIDENCE_LEVEL, Properties::get().MARGIN_OF_ERROR, testtriplesVector.size());
			testtriplesVector = t.getTesttriplesSample(testtriplesVector);

			FILE* test_sample_file;
			fopen_s(&test_sample_file, Properties::get().PATH_TEST_SAMPLE.c_str(), "w");

			for (auto triple : testtriplesVector) {
				fprintf(test_sample_file, "%s\t%s\t%s\n", index->getStringOfNodeId(*triple[0])->c_str(), index->getStringOfRelId(*triple[1])->c_str(), index->getStringOfNodeId(*triple[2])->c_str());
			}

			fclose(test_sample_file);
			std::cout << "Written test sample to " << Properties::get().PATH_TEST_SAMPLE << std::endl;
		}

		testtripleSize = new int;
		*testtripleSize = testtriplesVector.size();
		// Convert to pointers of pointers
		int * testtriplesstore;
		testtriples = new int*[*testtripleSize];
		testtriplesstore = new int[(*testtripleSize) * 3];

		for (int i = 0; i < (*testtripleSize); i++) {
			testtriplesstore[i * 3] = *(testtriplesVector[i][0]);
			testtriplesstore[i * 3 + 1] = *(testtriplesVector[i][1]);
			testtriplesstore[i * 3 + 2] = *(testtriplesVector[i][2]);
			testtriples[i] = &testtriplesstore[i * 3];
		}
	}
	else {
		std::cout << "Unable to open test file " << filepath << std::endl;
		exit(-1);
	}
}

#include "ValidationtripleReader.h"

ValidationtripleReader::ValidationtripleReader(std::string filepath, Index * index, TraintripleReader* graph) {
	this->index = index;
	this->graph = graph;
	read(filepath);
}

CSR<int, int> * ValidationtripleReader::getCSR() {
	return csr;
}

RelNodeToNodes& ValidationtripleReader::getRelHeadToTails() {
	return relHeadToTails;
}

RelNodeToNodes& ValidationtripleReader::getRelTailToHeads() {
	return relTailToHeads;
}

void ValidationtripleReader::read(std::string filepath) {
	std::string line;
	std::ifstream myfile(filepath);
	if (myfile.is_open())
	{
		std::vector<std::vector<int*>> testtriplesVector;
		while (!util::safeGetline(myfile, line).eof())
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
				RelTuples[*relId].push_back(std::make_pair(*headId, *tailId));
			} catch(std::runtime_error& e){}
		}
		myfile.close();

		csr = new CSR<int, int>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
	}
	else {
		std::cout << "Unable to open validation file " << filepath << std::endl;
		exit(-1);
	}
}


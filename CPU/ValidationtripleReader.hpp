#ifndef VALIDATIONTRIPLEREADER_H
#define VALIDATIONTRIPLEREADER_H

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "Types.h"
#include "CSR.hpp"
#include "Graph.hpp"
#include "Util.hpp"


class ValidationtripleReader
{

public:
	ValidationtripleReader(std::string filepath, Index * index, Graph * graph) {
		this->index = index;
		this->graph = graph;
		read(filepath);
	}

	CSR<int, int> * getCSR() {
		return csr;
	}

protected:

private:

	Index * index;
	Graph * graph;
	CSR<int, int> * csr;


	void read(std::string filepath) {
		RelNodeToNodes relHeadToTails;
		RelNodeToNodes relTailToHeads;
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
				int * headId = index->getIdOfNodestring(results[0]);
				int * relId = index->getIdOfRelationstring(results[1]);
				int * tailId = index->getIdOfNodestring(results[2]);

				std::vector<int*> testtriple;
				testtriple.push_back(headId);
				testtriple.push_back(relId);
				testtriple.push_back(tailId);
				testtriplesVector.push_back(testtriple);
				relHeadToTails[*relId][*headId].insert(*tailId);
				relTailToHeads[*relId][*tailId].insert(*headId);
			}
			myfile.close();

			csr = new CSR<int, int>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
		}
		else {
			std::cout << "Unable to open validation file " << filepath << std::endl;
			exit(-1);
		}
	}
};

#endif //VALIDATIONTRIPLEREADER_H


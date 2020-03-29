#ifndef TESTTRIPLEREADER_H
#define TESTTRIPLEREADER_H

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include "Util.hpp"
#include "Graph.hpp"
#include "Types.h"
#include "CSR.hpp"


class TesttripleReader
{

    public:
		TesttripleReader(std::string filepath, Index * index, Graph * graph) {
			this->index = index;
			this->graph = graph;
			read(filepath);
		}

		int ** getTesttriples() {
			return testtriples;
		}

		int * getTesttriplesSize() {
			return testtripleSize;
		}

		CSR<int, int> * getCSR() {
			return csr;
		}

		std::vector<int>& getUniqueRelations() {
			return uniqueRelations;
		}

		RelNodeToNodes& getRelHeadToTails() {
			return relHeadToTails;
		}

		RelNodeToNodes& getRelTailToHeads() {
			return relTailToHeads;
		}

    protected:

    private:
        int ** testtriples;
		int * testtripleSize;
		Index * index;
		Graph * graph;
		std::vector<int> uniqueRelations;
		CSR<int, int> * csr;

		RelNodeToNodes relHeadToTails;
		RelNodeToNodes relTailToHeads;


		void read(std::string filepath) {

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

				std::vector<int> keys(relHeadToTails.size());
				std::transform(relHeadToTails.begin(), relHeadToTails.end(), keys.begin(), [](auto pair) {return pair.first;});
				uniqueRelations = keys;

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
};

#endif // TESTTRIPLEREADER_H

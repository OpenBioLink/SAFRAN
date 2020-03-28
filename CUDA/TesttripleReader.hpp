#ifndef TESTTRIPLEREADER_H
#define TESTTRIPLEREADER_H

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include "Util.hpp"
#include "Graph.hpp"

#define cudaCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                msg, cudaGetErrorString(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

class TesttripleReader
{
    public:
		__host__ TesttripleReader(std::string filepath, Graph * graph, bool cuda) {
			this->graph = graph;
			read(filepath, cuda);
		}

		__host__ int ** getTesttriples() {
			return testtriples;
		}

		__host__ int * getTesttriplesSize() {
			return testtripleSize;
		}
    protected:

    private:
        int ** testtriples;
		int * testtripleSize;
		Graph * graph;
		__host__ void read(std::string filepath, bool cuda) {
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
						throw "Unsupported Filetype";
					}
					int * headId = graph->getIdOfNodestring(results[0]);
					int * relId = graph->getIdOfRelationstring(results[1]);
					int * tailId = graph->getIdOfNodestring(results[2]);
					if (headId == nullptr) {
						std::cout << "NUll" << std::endl;
					}
					if (relId == nullptr) {
						std::cout << "NUll" << std::endl;
					}
					if (tailId == nullptr) {
						std::cout << "NUll" << std::endl;
					}
					std::vector<int*> testtriple;
					testtriple.push_back(headId);
					testtriple.push_back(relId);
					testtriple.push_back(tailId);
					testtriplesVector.push_back(testtriple);
				}
				myfile.close();

				if (cuda) {
					cudaMallocManaged(&testtripleSize, sizeof(int));
				}
				else {
					testtripleSize = new int;
				}
				*testtripleSize = testtriplesVector.size();
				// Convert to pointers of pointers
				int * testtriplesstore;
				if (cuda) {
					cudaMallocManaged(&testtriples, (*testtripleSize) * sizeof(int*));
					cudaCheckErrors("memc testtriples");
					cudaMallocManaged(&testtriplesstore, 3 * (*testtripleSize) * sizeof(int));
					cudaCheckErrors("memc testtriplesstore");
				}
				else {
					testtriples = new int*[*testtripleSize];
					testtriplesstore = new int[(*testtripleSize) * 3];
				}
				for (int i = 0; i < (*testtripleSize); i++) {
					testtriplesstore[i * 3] = *(testtriplesVector[i][0]);
					testtriplesstore[i * 3 + 1] = *(testtriplesVector[i][1]);
					testtriplesstore[i * 3 + 2] = *(testtriplesVector[i][2]);
					testtriples[i] = &testtriplesstore[i * 3];
				}

			}
			else std::cout << "Unable to open file";
		}
};

#endif // TESTTRIPLEREADER_H

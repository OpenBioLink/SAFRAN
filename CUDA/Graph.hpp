#ifndef GRAPH_H
#define GRAPH_H

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>

#include "Util.hpp"

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

class Graph
{

	typedef std::unordered_set<int> Nodes;
	typedef std::unordered_map<int, Nodes> NodeToNodes;
	typedef std::unordered_map<int, NodeToNodes> RelNodeToNodes;

    public:
		__host__ Graph(std::string filepath, bool cuda) {
			nodeToId = new std::unordered_map<std::string, int>();
			relToId = new std::unordered_map<std::string, int>();

			read(filepath);
			nodeToId->rehash(nodeToId->size());
			relToId->rehash(relToId->size());

			for (int i = 0; i < relToId->size(); i++) {
				relHeadToTails[i].rehash(relHeadToTails[i].size());
				relTailToHeads[i].rehash(relTailToHeads[i].size());
			}
			relHeadToTails.rehash(relHeadToTails.size());
			relTailToHeads.rehash(relTailToHeads.size());

			generateCSR(cuda);
		}

		__host__ std::unordered_map<std::string, int>* getNodeToId() {
			return nodeToId;
		}

		__host__ std::unordered_map<std::string, int>* getRelToId() {
			return relToId;
		}

		__host__ int* getIdOfNodestring(std::string& node) {
			try {
				return &(nodeToId->at(node));
			}
			catch (const std::out_of_range&) {
				return nullptr;
			}
		}

		__host__ int* getIdOfRelationstring(std::string& relation) {
			try {
				return &(relToId->at(relation));
			}
			catch (const std::out_of_range&) {
				return nullptr;
			}
		}

		__host__ void generateCSR(bool cuda) {
			int numRel = relToId->size();
			int numNodes = nodeToId->size();
			int nnz = 0;
			int * indlen = new int[numRel * 2];
			for (int i = 0; i < numRel; i++) {
				int indlenHTT = 0;
				int indlenTTH = 0;
				for (int j = 0; j < numNodes; j++) {
					if (relHeadToTails.find(i) != relHeadToTails.end()) {
						if (relHeadToTails[i].find(j) != relHeadToTails[i].end()) {
							indlenHTT = indlenHTT + relHeadToTails[i][j].size();
						}
					}
					if (relTailToHeads.find(i) != relTailToHeads.end()) {
						if (relTailToHeads[i].find(j) != relTailToHeads[i].end()) {
							indlenTTH = indlenTTH + relTailToHeads[i][j].size();
						}
					}
				}
				indlen[i * 2] = indlenHTT;
				indlen[i * 2 + 1] = indlenTTH;
				nnz += indlenHTT;
				nnz += indlenTTH;
			}
			// Size is the number of relations times 2, because the inverse is included, times (3 + numNodes). 3 because [length, length of indptr, length of ind (NNZ)].
			// Plus the number of all non zero values.
			int * h_adj_lists = new int[numRel * 2 * (3 + numNodes + 1) + nnz];
			int * h_adj_list_starts = new int [numRel * 2];

			int start = 0;
			for (int rel = 0; rel < numRel; rel++) {

				h_adj_list_starts[rel * 2] = start;
				int currentIndptrlen = numNodes + 1;
				int currentIndlen = indlen[rel * 2];
				int currentLen = 3 + currentIndptrlen + currentIndlen;

				int stop = start + currentLen;

				h_adj_lists[start] = currentLen;
				h_adj_lists[start + 1] = currentIndptrlen;
				h_adj_lists[start + 2] = currentIndlen;

				int * indptr = &h_adj_lists[start + 3];
				int * ind = &h_adj_lists[start + 3 + currentIndptrlen];

				*indptr = 0;
				for (int node = 0; node < numNodes; node++) {
					if (relHeadToTails.find(rel) != relHeadToTails.end()) {
						if (relHeadToTails[rel].find(node) != relHeadToTails[rel].end()) {
							*(indptr + 1) = *indptr + relHeadToTails[rel][node].size();
							indptr++;
							auto it = relHeadToTails[rel][node].begin();
							while (it != relHeadToTails[rel][node].end())
							{
								*ind = *it;
								ind++;
								it++;
							}
						}
						else {
							*(indptr + 1) = *indptr;
							indptr++;
						}
					}
				}
				start = stop;


				h_adj_list_starts[rel * 2 + 1] = start;
				currentIndptrlen = numNodes + 1;
				currentIndlen = indlen[rel * 2 + 1];
				currentLen = 3 + currentIndptrlen + currentIndlen;

				stop = start + currentLen;

				h_adj_lists[start] = currentLen;
				h_adj_lists[start + 1] = currentIndptrlen;
				h_adj_lists[start + 2] = currentIndlen;

				indptr = &h_adj_lists[start + 3];
				ind = &h_adj_lists[start + 3 + currentIndptrlen];

				*indptr = 0;
				for (int node = 0; node < numNodes; node++) {
					if (relTailToHeads.find(rel) != relTailToHeads.end()) {
						if (relTailToHeads[rel].find(node) != relTailToHeads[rel].end()) {
							*(indptr + 1) = *indptr + relTailToHeads[rel][node].size();
							indptr++;
							auto it = relTailToHeads[rel][node].begin();
							while (it != relTailToHeads[rel][node].end())
							{
								*ind = *it;
								ind++;
								it++;
							}
						}
						else {
							*(indptr + 1) = *indptr;
							indptr++;
						}
					}
				}
				start = stop;
			}

			
			cudaMalloc(&adj_lists, (numRel * 2 * (3 + numNodes + 1) + nnz) * sizeof(int));
			cudaCheckErrors("mall adj_lists");
			cudaMemcpy(adj_lists, h_adj_lists, (numRel * 2 * (3 + numNodes + 1) + nnz) * sizeof(int), cudaMemcpyHostToDevice);
			cudaCheckErrors("memc adj_lists");

			cudaMalloc(&adj_list_starts, numRel * 2 * sizeof(int));
			cudaCheckErrors("cm adj_lists_starts");
			cudaMemcpy(adj_list_starts, h_adj_list_starts, numRel * 2 * sizeof(int), cudaMemcpyHostToDevice);
			cudaCheckErrors("cmem adj_lists_starts");

			delete[] h_adj_lists;
			delete[] h_adj_list_starts;
		}

		int* getAdjLists() {
			return adj_lists;
		}

		int* getAdjListStarts() {
			return adj_list_starts;
		}
    protected:

    private:
		int* adj_lists;
		int* adj_list_starts;
        int maxNodeID = 0;
        int maxRelID = 0;
		RelNodeToNodes relHeadToTails;
		RelNodeToNodes relTailToHeads;
        std::unordered_map<std::string, int>* nodeToId;
        std::unordered_map<std::string, int>* relToId;

		__host__ void read(std::string filepath) {
			std::string line;
			std::ifstream myfile(filepath);
			if (myfile.is_open())
			{
				while (getline(myfile, line))
				{
					std::istringstream iss(line);
					std::vector<std::string> results = util::split(line, '\t');
					if (results.size() != 3) {
						throw "Unsupported Filetype";
					}
					add(results[0], results[1], results[2]);
				}
				myfile.close();
			}
			else std::cout << "Unable to open file";
		}

		__host__ void add(std::string head, std::string relation, std::string tail) {
			//Get ids
			if (nodeToId->count(head) == 0) {
				std::pair<std::string, int> ins(head, maxNodeID);
				nodeToId->insert(ins);
				maxNodeID++;
			}
			if (nodeToId->count(tail) == 0) {
				std::pair<std::string, int> ins(tail, maxNodeID);
				nodeToId->insert(ins);
				maxNodeID++;
			}
			if (relToId->count(relation) == 0) {
				std::pair<std::string, int> ins(relation, maxRelID);
				relToId->insert(ins);
				maxRelID++;
			}
			int headNodeId = nodeToId->at(head);
			int relId = relToId->at(relation);
			int tailNodeId = nodeToId->at(tail);

			relHeadToTails[relId][headNodeId].insert(tailNodeId);
			relTailToHeads[relId][tailNodeId].insert(headNodeId);
		}
};

#endif // GRAPH_H

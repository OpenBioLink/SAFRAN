#define N_TRIAL 100000
#define N_SAMPLES 1000

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cuda_profiler_api.h>
#include <stdio.h>
#include <ctime>

#include <thrust/unique.h>
#include <thrust/sort.h>
#include <thrust/execution_policy.h>
#include <thrust/set_operations.h>

#include "Graph.hpp"
#include "RuleReader.hpp"
#include "TesttripleReader.hpp"
#include "ScoreTree.hpp"
#include "Rule.hpp"

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

__device__ void applyCyclicRule(int* adj_list_starts, int* adj_lists, int * val, int* relations, int rulelength, int N, int * result, int * resultlength, int& counter, int * previous) {
	counter++;
	if (counter == N_TRIAL) {
		return;
	}
	int value = *val;
	previous[N] = value;
	int * adj_list = &(adj_lists[adj_list_starts[*relations]]);
	int start_indptr = 3;
	int size_indptr = adj_list[1];
	int start_ind = start_indptr + size_indptr;
	//int size_ind = adj_list[2];

	relations++;
	N++;
	int ind_ptr = adj_list[start_indptr + value];
	int len = adj_list[start_indptr + value + 1] - ind_ptr;
	if (N == rulelength) {
		for (int j = 0; j < len; j++)
		{
			int nextval = adj_list[start_ind + ind_ptr + j];
			for (int i = 0; i < N; i++) {
				if (previous[i] == nextval) {
					break;
				}
			}
			result[*resultlength] = nextval;
			(*resultlength)++;
			if (*resultlength == N_SAMPLES) return;
			counter++;
			if (counter == N_TRIAL) {
				return;
			}
		}
	}
	else {
		for (int j = 0; j < len; j++)
		{
			int nextval = adj_list[start_ind + ind_ptr + j];
			for (int i = 0; i < N; i++) {
				if (previous[i] == nextval) {
					break;
				}
			}
			applyCyclicRule(adj_list_starts, adj_lists, &nextval, relations, rulelength, N, result, resultlength, counter, previous);
			if (*resultlength == N_SAMPLES) return;
		}
	}
}
__device__ void applyAcyclicRule(int* adj_list_starts, int* adj_lists, int* relations, int rulelength, int N, int * result, int * resultlength, int& counter, int * previous) {
	int * adj_list = &(adj_lists[adj_list_starts[*relations]]);
	int start_indptr = 3;
	int size_indptr = adj_list[1];
	int start_ind = start_indptr + size_indptr;
	//int size_ind = adj_list[2];

	relations++;
	N++;

	if (N == rulelength) {
		for (int val = 0; val < size_indptr; val++) {
			previous[N - 1] = val;
			int ind_ptr = adj_list[start_indptr + val];
			int len = adj_list[start_indptr + val + 1] - ind_ptr;
			for (int j = 0; j < len; j++) {
				int nextval = adj_list[start_ind + ind_ptr + j];
				for (int i = 0; i < N; i++) {
					if (previous[i] == nextval) {
						break;
					}
				}
				result[*resultlength] = nextval;
				(*resultlength)++;
				if (*resultlength == N_SAMPLES) return;
				counter++;
				if (counter == N_TRIAL) {
					return;
				}
			}
		}
	}
	else {
		for (int val = 0; val < size_indptr; val++) {
			previous[N - 1] = val;
			int ind_ptr = adj_list[start_indptr + val];
			int len = adj_list[start_indptr + val + 1] - ind_ptr;
			for (int j = 0; j < len; j++) {
				int nextval = adj_list[start_ind + ind_ptr + j];
				for (int i = 0; i < N; i++) {
					if (previous[i] == nextval) {
						break;
					}
				}
				applyCyclicRule(adj_list_starts, adj_lists, &nextval, relations, rulelength, N, result, resultlength, counter, previous);
				if (*resultlength == N_SAMPLES) return;
			}
		}
	}
}
__device__ bool existsAcyclic(int* adj_list_starts, int* adj_lists, int * valId, int * constant, int* relations, int N) {
	int * adj_list = &(adj_lists[adj_list_starts[*relations]]);
	int start_indptr = 3;
	int size_indptr = adj_list[1];
	int start_ind = start_indptr + size_indptr;
	//int size_ind = adj_list[2];

	relations++;
	N--;
	int ind_ptr = adj_list[start_indptr + *valId];
	int len = adj_list[start_indptr + *valId + 1] - ind_ptr;
	if (N == 0 && constant != nullptr) {
		for (int j = 0; j < len; j++) {
			int to = adj_list[start_ind + ind_ptr + j];
			if (to == *constant) {
				return true;
			}
		}
		return false;
	}
	else if (N == 0 && constant == nullptr) {
		if (len > 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		for (int j = 0; j < len; j++) {
			if (existsAcyclic(adj_list_starts, adj_lists, &adj_list[start_ind + ind_ptr + j], constant, relations, N)) {
				return true;
			};
		}
		return false;
	}
}

__device__ void computeHeadsCyclic(int* adj_list_starts, int* adj_lists, int * valId, Rule& rule, int * result, int * resultlength, int * previous) {
	int* relations = rule.getRelationsBwd();
	int counter = 0;
	applyCyclicRule(adj_list_starts, adj_lists, valId, relations, rule.getRulelength(), 0, result, resultlength, counter, previous);
}
__device__ void computeTailsCyclic(int* adj_list_starts, int* adj_lists, int * valId, Rule& rule, int * result, int * resultlength, int * previous) {
	int* relations = rule.getRelationsFwd();
	int counter = 0;
	applyCyclicRule(adj_list_starts, adj_lists, valId, relations, rule.getRulelength(), 0, result, resultlength, counter, previous);
}
__device__ void computeAcyclic(int* adj_list_starts, int* adj_lists, Rule& rule, int * result, int * resultlength, int * previous) {
	int* relations = rule.getRelationsBwd();
	int counter = 0;
	if (*(rule.getBodyconstantId()) != INT_MAX) {
		applyCyclicRule(adj_list_starts, adj_lists, rule.getBodyconstantId(), relations, rule.getRulelength(), 0, result, resultlength, counter, previous);
	}
	else {
		applyAcyclicRule(adj_list_starts, adj_lists, relations, rule.getRulelength(), 0, result, resultlength, counter, previous);
	}
}
__device__ bool existsAcyclic(int* adj_list_starts, int* adj_lists, int * valId, Rule& rule) {
	int* relations = rule.getRelationsFwd();
	int * constantnode = nullptr;
	if (*(rule.getBodyconstantId()) != INT_MAX) {
		constantnode = rule.getBodyconstantId();
	}
	return existsAcyclic(adj_list_starts, adj_lists, valId, constantnode, relations, rule.getRulelength());
}

__global__
void run(int* adj_list_starts, int* adj_lists, int * rulesAdjBegin, Rule * rulesAdjList, int** testtriples, int * testtriplessize, int ** results, int * resultlengths, int ** previous, ScoreTree * sts) {
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int stride = blockDim.x * gridDim.x;

	int * adj_begin = rulesAdjBegin;
	Rule * rules_adj_list = rulesAdjList;
	//Foreach testtriples
	for (int i = index; index < *testtriplessize; i += stride) {
		sts[i].initializeScoreTree();

		int * testtriple = testtriples[i];
		int ind_ptr = adj_begin[3 + testtriple[1]];
		int len = adj_begin[3 + testtriple[1] + 1] - ind_ptr;
		// Foreach rule
		for (int j = 0; j < len; j++) {
			Rule currRule = rules_adj_list[ind_ptr + j];
			//printf("Rule ");
			//currRule.print();
			//printf("\n");
			//currRule.toString();
			//std::cout << "Testtriple: " << *testtriple[0] << std::endl;
			if (currRule.getRuletype() == Ruletype::XRule) {
				if (existsAcyclic(adj_list_starts, adj_lists, &testtriple[0], currRule)) {
					results[i][resultlengths[i]] = testtriple[0];
				}
				//std::cout << "exists: " << x << std::endl;
			}
			else if (currRule.getRuletype() == Ruletype::YRule) {
				if (*currRule.getHeadconstant() == testtriple[0]) {
					computeAcyclic(adj_list_starts, adj_lists, currRule, results[i], &resultlengths[i], previous[i]);
					/*
					std::cout << "Reslen: " << *resultlength << std::endl;
					for (int a = 0; a < *resultlength; a++) {
						std::cout <<"acsol: "<< *results[a] << std::endl;
					}
					*/
				}
			}
			else {
				computeTailsCyclic(adj_list_starts, adj_lists, &testtriple[0], currRule, results[i], &resultlengths[i], previous[i]);
				/*
				std::cout << "Reslen: " << *resultlength << std::endl;
				for (int a = 0; a < *resultlength; a++) {
					std::cout << "cycsol: " << *results[a] << std::endl;
				}
				*/
			}


			//Unique values of results
			if (resultlengths[i] > 0) {
				thrust::sort(thrust::device, results[i], (results[i] + resultlengths[i]));
				int * end = thrust::unique(thrust::device, results[i], (results[i] + resultlengths[i]));
				//Filter results from trainingsset
				int * adj_list = &(adj_lists[adj_list_starts[testtriple[1]]]);
				int * indptr = &adj_list[3 + testtriple[0]];
				int len = *(indptr + 1) - *indptr;
				int * ind = &adj_list[3 + adj_list[1] + *indptr];
				thrust::sort(thrust::device, ind, ind + len);
				int * endDiff = thrust::set_difference(thrust::device, results[i], end, ind, ind + len, results[i]);
				int nValues = thrust::distance(results[i], endDiff);

				sts[i].addValues(currRule.getAppliedConfidence(), results[i], nValues);
				resultlengths[i] = 0;
			}

		}
		sts[i].Free();
	}
}

int main()
{
	size_t limit = 7000000000;
	cudaDeviceSetLimit(cudaLimitMallocHeapSize,limit); 
	unsigned int intermediate = clock();

	//"C:\\Users\\Simon\\Desktop\\Parullel_Cuda_wAdj\\train.txt"
	Graph * graph = new Graph("/home/eisernried12/train.txt", true);
	int * nodesAdjLists = graph->getAdjLists();
	int * nodesAdjListStarts = graph->getAdjListStarts();
	printf("Graph created in millisecs: %ld\n", clock() - intermediate);
	intermediate = clock();

	
	//"C:\\Users\\Simon\\Desktop\\Parullel_Cuda_wAdj\\rules.txt"
	RuleReader rr = RuleReader("/home/eisernried12/alpha-50", graph, true);
	int * rulesAdjBegin = rr.getRuleAdjBegin();
	Rule * rulesAdjList = rr.getRulesAdjList();
	printf("Rules created in millisecs: %ld\n", clock() - intermediate);
	intermediate = clock();

	//"C:\\Users\\Simon\\Desktop\\Parullel_Cuda_wAdj\\test.txt"
	TesttripleReader ttr = TesttripleReader("/home/eisernried12/test.txt", graph,true);
	int ** testtriples = ttr.getTesttriples();
	int * testtriplesSize = ttr.getTesttriplesSize();
	printf("Testtriples created in millisecs: %ld\n", clock() - intermediate);
	intermediate = clock();

	int ** results;
	int * resultlengths;
	int ** previous;
	ScoreTree * sts;
	cudaMallocManaged(&results, *testtriplesSize * sizeof(int*));
	cudaMallocManaged(&resultlengths, *testtriplesSize * sizeof(int));
	cudaMallocManaged(&previous, *testtriplesSize * sizeof(int*));
	cudaMallocManaged(&sts, *testtriplesSize * sizeof(ScoreTree));
	for (int i = 0; i < *testtriplesSize; i++) {
		cudaMallocManaged(&results[i], N_SAMPLES * sizeof(int));
		cudaMallocManaged(&previous[i], 10 * sizeof(int));
		resultlengths[i] = 0;
	}
	cudaCheckErrors("ScoreTree");

	int blockSize = 256;
	int numBlocks = (*testtriplesSize + blockSize - 1) / blockSize;

	std::cout << "START" << std::endl;
	run <<<numBlocks, blockSize>>> (nodesAdjListStarts, nodesAdjLists, rulesAdjBegin, rulesAdjList, testtriples, testtriplesSize, results, resultlengths, previous, sts);
	
	cudaDeviceSynchronize();
	
	std::cout << "Rules applied in millisecs: " << clock() - intermediate << std::endl;
	cudaDeviceReset();
	
	return 0;
}

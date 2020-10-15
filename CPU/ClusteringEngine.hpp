#ifndef CLUSTERENGINE_H
#define CLUSTERENGINE_H

#include "RuleReader.hpp"
#include "Index.hpp"
#include "Graph.hpp"
#include "Clustering.hpp"
#include <iostream>
#include "RuleGraph.hpp"
#include <omp.h>

class ClusteringEngine {

public:

	ClusteringEngine(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
		this->index = index;
		this->graph = graph;
		this->ttr = ttr;
		this->vtr = vtr;
		this->rr = rr;
		adj_lists = graph->getCSR()->getAdjList();
		adj_list_starts = graph->getCSR()->getAdjBegin();
		vt_adj_lists = vtr->getCSR()->getAdjList();
		vt_adj_list_starts = vtr->getCSR()->getAdjBegin();
		rules_adj_list = rr->getCSR()->getAdjList();
		adj_begin = rr->getCSR()->getAdjBegin();
		WORKER_THREADS = Properties::get().WORKER_THREADS;
		this->lock = new std::mutex();
		this->writeLock = new std::mutex();
		this->rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
	}

	FILE* pFile;

	void learn() {
		fopen_s(&pFile, Properties::get().PATH_CLUSTER.c_str(), "w");

		_learn(0);

		fclose(pFile);
	}

	void calc_sols(Clustering* cluster, int ind_ptr, int len) {
		std::cout << Properties::get().CLUSTER_SET << "\n";
		int size = index->getNodeSize();

#pragma omp parallel for schedule(dynamic)
		for(int j = 0; j < len; j++){
			
			std::vector<std::vector<int>> heads;
			std::vector<std::vector<int>> tails;

			Rule currRule = rules_adj_list[ind_ptr + j]; 
			
			if (currRule.is_ac1()) {
				cluster->addSolution(currRule, j, heads, tails);
				continue;
			}

			int rulelength = currRule.getRulelength();
			bool** visited = new bool* [rulelength];
			for (int i = 0; i < rulelength; i++) {
				visited[i] = new bool[size];
				std::fill(visited[i], visited[i] + size, false);
			}
			int* previous = new int[rulelength];

			

			if (j % 100 == 0) {
				std::cout << j << "\n";
			}
			if (currRule.getRuletype() == Ruletype::XRule) {
				if (currRule.getBodyconstantId() != nullptr) {
					std::vector<int> results;
					rulegraph->searchDFSSingleStart(*currRule.getBodyconstantId(), currRule, true, results, previous, visited);
					heads.push_back(results);
					tails.push_back(std::vector<int> {*currRule.getHeadconstant()});
				}
				else {
					std::vector<int> results;
					//rulegraph->searchDFSMultiStart(currRule, true, results);
					heads.push_back(results);
					tails.push_back(std::vector<int> {*currRule.getHeadconstant()});
				}
			}
			// ALL of Y are [head] | tail1, tail2, ...
			else if (currRule.getRuletype() == Ruletype::YRule) {
				if (currRule.getBodyconstantId() != nullptr) {
					std::vector<int> results;
					rulegraph->searchDFSSingleStart(*currRule.getBodyconstantId(), currRule, true, results, previous, visited);
					heads.push_back(std::vector<int> {*currRule.getHeadconstant()});
					tails.push_back(results);
				}
				else {
					std::vector<int> results;
					//rulegraph->searchDFSMultiStart(currRule, true, results);
					heads.push_back(std::vector<int> {*currRule.getHeadconstant()});
					tails.push_back(results);
				}
			}
			else {
				int* relations = currRule.getRelationsFwd();
				int* adj_list;
				if (Properties::get().CLUSTER_SET.compare("train") == 0) {
					adj_list = &(adj_lists[adj_list_starts[*relations]]);
				}
				else {
					adj_list = &(vt_adj_lists[vt_adj_list_starts[*relations]]);
				}
				int start_indptr = 3;
				int size_indptr = adj_list[1];
				int start_ind = start_indptr + size_indptr;
				for (int val = 0; val < size_indptr - 1; val++) {
					int ind_ptr = adj_list[start_indptr + val];
					int len = adj_list[start_indptr + val + 1] - ind_ptr;
					if (len > 0) {
						std::vector<int> results;
						rulegraph->searchDFSSingleStart(val, currRule, false, results, previous, visited);
						if (results.size() > 0) {
							heads.push_back(std::vector<int> {val});
							tails.push_back(results);
						}
						for (int i = 0; i < rulelength; i++) {
							std::fill(visited[i], visited[i] + size, false);
						}
					}
				}
			}
			for (int i = 0; i < rulelength; i++) {
				delete[] visited[i];
			}
			delete[] visited;
			delete[] previous;
			cluster->addSolution(currRule, j, heads, tails);

			

			//std::cout << currRule.getRuletype() << " " << *headresultlength << " " << *tailresultlength << " " << currRule.getRulelength() << std::endl;

			
		}
	}

	void _learn(int threadId) {

		int rellen = index->getRelSize();
		for(int i = 0; i < rellen; i++){
			rule = -1;
			int ind_ptr = adj_begin[3 + i];
			int len = adj_begin[3 + i + 1] - ind_ptr;

			Clustering* cluster = new Clustering(200, i, len, index, graph, ttr, vtr, rr);

			std::cout << "Cluster calculating " << *(index->getStringOfRelId(i)) << " " << len << std::endl;

			calc_sols(cluster, ind_ptr, len);
			std::cout << "Calced sols\n";

			auto result = cluster->learn_cluster();

			writeThresh(result);

			delete cluster;
			std::cout << "Cluster calculated for " << i << "/" << rellen << " rule relations" << std::endl;
		}

		std::cout << "DONEZO" << std::endl;
	}

	typedef std::function<bool(std::pair<int, double>, std::pair<int, double>)> Comparator;
	Comparator compFunctor =
		[](std::pair<int, double> elem1, std::pair<int, double> elem2)
	{
		return elem1.second > elem2.second;
	};


private:
	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;
	RuleReader* rr;
	int WORKER_THREADS;

	int* adj_lists;
	int* adj_list_starts;

	int* vt_adj_lists;
	int* vt_adj_list_starts;

	Rule* rules_adj_list;
	int* adj_begin;

	std::mutex* lock;
	int rule = -1;

	RuleGraph* rulegraph;

	int getNextRule() {
		int rel;
		lock->lock();
		rule++;
		rel = rule;
		lock->unlock();
		return rel;
	}

	std::mutex* writeLock;
	void writeThresh(std::string result) {
		writeLock->lock();
		fprintf(pFile, "%s", result.c_str());
		fflush(pFile);
		writeLock->unlock();
	}

	/*
	std::mutex* writeLock;
	void writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, double>> headresults, std::vector<std::pair<int, double>> tailresults, FILE* pFile, int& K) {
		writeLock->lock();
		fprintf(pFile, "%s %s %s\nHeads: ", index->getStringOfNodeId(head)->c_str(), index->getStringOfRelId(rel)->c_str(), index->getStringOfNodeId(tail)->c_str());

		int maxHead = headresults.size() < K ? headresults.size() : K;
		for (int i = 0; i < maxHead; i++) {
			fprintf(pFile, "%s\t%.16f\t", index->getStringOfNodeId(headresults[i].first)->c_str(), headresults[i].second);
		}
		fprintf(pFile, "\nTails: ");
		int maxTail = tailresults.size() < K ? tailresults.size() : K;
		for (int i = 0; i < maxTail; i++) {
			fprintf(pFile, "%s\t%.16f\t", index->getStringOfNodeId(tailresults[i].first)->c_str(), tailresults[i].second);
		}
		fprintf(pFile, "\n");
		writeLock->unlock();
	}
	*/

};

#endif // CLUSTERENGINE_H

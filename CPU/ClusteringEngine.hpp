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
		this->writeLock = new std::mutex();
	}

	FILE* pFile;

	void learn() {
		fopen_s(&pFile, Properties::get().PATH_CLUSTER.c_str(), "w");

		_learn(0);

		fclose(pFile);
	}

	void _learn(int threadId) {

		int rellen = index->getRelSize();
		for(int i = 0; i < rellen; i++){
			std::cout << "Calculating cluster for " << *index->getStringOfRelId(i) << " " << rellen << "\n";
			int ind_ptr = adj_begin[3 + i];
			int len = adj_begin[3 + i + 1] - ind_ptr;

			Clustering* cluster = new Clustering(200, i, len, index, graph, ttr, vtr, rr);

			std::string jacc_file = Properties::get().PATH_JACCARD + std::string("/") + std::to_string(i) + std::string("_jacc.bin");
			auto result = cluster->learn_cluster(jacc_file);

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

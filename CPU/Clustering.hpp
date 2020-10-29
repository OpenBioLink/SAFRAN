#ifndef CL_H
#define CL_H

#include <algorithm>
#include <random>
#include <unordered_set>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <fstream>
#include <random>
#include "Graph.hpp"
#include "MinHash.hpp"

class Clustering {
public:
	Clustering(int portions, int relation, int size, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr);
	std::string learn_cluster(std::string jacc_path);
private:
	int portions;
	uint32_t samples_size;
	uint32_t features_size;
	int relation;

	int k = 200;

	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;

	RuleReader* rr;
	int* adj_begin;
	Rule* rules_adj_list;
	int ind_ptr;
	int lenRules;

	int WORKER_THREADS;

	static bool sortbysec(const std::pair<int, double>& a, const std::pair<int, double>& b)
	{
		return (a.second >= b.second);
	}

	std::vector<std::pair<int, double>>* read_jaccard(std::string path);
	void learn_parameters(Graph* g, RuleGraph* rulegraph, double* res, std::vector<std::vector<int>>* res_clusters);
};

Clustering::Clustering(int portions, int relation, int size, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->portions = portions;
	this->index = index;
	this->graph = graph;
	this->ttr = ttr;
	this->vtr = vtr;
	this->rr = rr;
	adj_begin = rr->getCSR()->getAdjBegin();
	rules_adj_list = rr->getCSR()->getAdjList();
	ind_ptr = adj_begin[3 + relation];
	lenRules = adj_begin[3 + relation + 1] - ind_ptr;
	this->relation = relation;
	samples_size = size;
	features_size = size;

	WORKER_THREADS = Properties::get().WORKER_THREADS;
}

std::string Clustering::learn_cluster(std::string jacc_path) {
	std::vector<std::pair<int, double>>* jacc = read_jaccard(jacc_path);
	Graph* g = new Graph(samples_size, jacc, rules_adj_list, ind_ptr);
	std::cout << "Calced jaccs\n";

	const unsigned long long MAX_BUF = Properties::get().BUFFER_SIZE;
	long long CURR_BUF = 0;
	bool stop = false;
	int asdf = 0;

	int* vt_adj_list = vtr->getCSR()->getAdjList();
	int* vt_adj_begin = vtr->getCSR()->getAdjBegin();

	RuleGraph* rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < lenRules; i++) {
		if (i % 100 == 0) { std::cout << i << "\n"; }
		Rule& currRule = rules_adj_list[ind_ptr + i];
		if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::XRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::YRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::XRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::YRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}

		if (currRule.is_c()) {
			if (stop) { 
				continue; 
			} else {
				#pragma omp atomic
				asdf++;
			}

			{
				int* v_adj_list = &(vt_adj_list[vt_adj_begin[relation * 2]]);
				if (vtr->getRelHeadToTails().find(relation) == vtr->getRelHeadToTails().end()) continue;
				for (auto heads = vtr->getRelHeadToTails()[relation].begin(); heads != vtr->getRelHeadToTails()[relation].end(); heads++) {
					int head = heads->first;
					int lenTails = heads->second.size();
					if (lenTails > 0) {
						std::vector<int> tailresults_vec;
						rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, false, true);
						if (tailresults_vec.size() + CURR_BUF < MAX_BUF) {
#pragma omp critical
							{
								if (tailresults_vec.size() + CURR_BUF < MAX_BUF) {
									currRule.setHeadBuffer(head, tailresults_vec);
									CURR_BUF = CURR_BUF + tailresults_vec.size();
								}
								else {
									stop = true;
								}
							}
						}
						else {
							stop = true;
						}
						if (stop) break;
					}
				}
			}

			{

				int* v_adj_list = &(vt_adj_list[vt_adj_begin[relation * 2 + 1]]);
				for (auto tails = vtr->getRelTailToHeads()[relation].begin(); tails != vtr->getRelTailToHeads()[relation].end(); tails++) {
					int tail = tails->first;
					int lenHeads = tails->second.size();
					if (lenHeads > 0) {
						std::vector<int> headresults_vec;
						rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, false, true);
						if (headresults_vec.size() + CURR_BUF < MAX_BUF) {
#pragma omp critical
							{
								if (headresults_vec.size() + CURR_BUF < MAX_BUF) {
									currRule.setTailBuffer(tail, headresults_vec);
									CURR_BUF = CURR_BUF + headresults_vec.size();
								}
								else {
									stop = true;
								}
							}
						}
						else {
							stop = true;
						}
						if (stop) break;
					}
				}	
			}
		}
	}
	std::cout << "DONE buffering " << CURR_BUF << "\n";

	double* res = new double[portions + 1];
	std::vector<std::vector<int>>* res_clusters = new std::vector<std::vector<int>>[portions + 1];
	learn_parameters(g, rulegraph, res, res_clusters);

	delete rulegraph;
	for (int i = 0; i < lenRules; i++) {
		Rule& currRule = rules_adj_list[ind_ptr + i];
		if (currRule.isBuffered()) {
			currRule.removeBuffer();
		}
		if (currRule.is_c()) {
			currRule.clearHeadBuffer();
			currRule.clearTailBuffer();
		}
	}

	std::cout << "Calced params\n";

	int max_param = 0;
	double max_thresh = 0.0;
	double max_mrr = 0.0;

	for (int i = 0; i < portions + 1; i++) {

		double thresh = (double)i / portions;

		if (res[i] > max_mrr) {
			max_param = i;
			max_thresh = thresh;
			max_mrr = res[i];
		}
	}

	std::cout << "MAX " << max_thresh << " " << max_mrr << "\n";
	std::ostringstream stringStream;
	stringStream << "Relation\t" << *index->getStringOfRelId(relation) << "\t" << max_thresh << " " << max_mrr << "\n";
	for (auto cluster : res_clusters[max_param]) {
		for (auto rule : cluster) {
			Rule& r = rules_adj_list[ind_ptr + rule];
			stringStream << r.getRulestring() << "\t";
		}
		stringStream << "\n";
	}
	stringStream << "\n";
	delete[] jacc;
	delete g;
	delete[] res;
	delete[] res_clusters;

	return stringStream.str();
}

std::vector<std::pair<int, double>>* Clustering::read_jaccard(std::string path) {
	int size = -1;
	std::vector<std::pair<int, double>>* jaccs_bin;

	std::ifstream myfile(path, std::ios::binary);

	if (myfile.is_open()) {
		int size;
		myfile.read((char*)(&size), sizeof size);
		jaccs_bin = new std::vector<std::pair<int, double>>[size];
		for (int i = 0; i < size; i++) {
			int len;
			myfile.read((char*)(&len), sizeof len);
			for (int j = 0; j < len; j++) {
				int r;
				double jacc;
				myfile.read((char*)(&r), sizeof r);
				myfile.read((char*)(&jacc), sizeof jacc);
				jaccs_bin[i].push_back(std::make_pair(r, jacc));
			}
		}
		myfile.close();
		return jaccs_bin;
	}
	else {
		std::cout << "Unable to open rule file " << path << std::endl;
		exit(-1);
	}
}

void Clustering::learn_parameters(Graph * g, RuleGraph * rulegraph, double* res, std::vector<std::vector<int>>* res_clusters) {
#pragma omp parallel for schedule(dynamic)
	for(int i = 0; i < portions+1; i++){
		NoisyOrEngine* noe = new NoisyOrEngine(relation, rulegraph, index, graph, ttr, vtr, rr);
		double thresh = (double)i / portions;

		std::cout << thresh << "\n";

		if (thresh == 0.0) {
			std::vector<std::vector<int>> clusters;
			std::vector<int> cluster;
			for (int i = 0; i < samples_size; i++) {
				Rule& r = rules_adj_list[ind_ptr + i];
				cluster.push_back(i);
			}
			clusters.push_back(cluster);

			std::cout << clusters[0].size() << "\n";

			
			auto result = noe->max(clusters);

			res[i] = result;
			res_clusters[i] = clusters;
			delete noe;
		} else {

			NoisyOrEngine* noe = new NoisyOrEngine(relation, rulegraph, index, graph, ttr, vtr, rr);
			std::vector<std::vector<int>> clusters;

			bool* visited = new bool[samples_size];

			for (int i = 0; i < samples_size; i++) {
				visited[i] = false;
			}

			for (int i = 0; i < samples_size; i++) {
				if (visited[i] == false) {
					visited[i] = true;
					std::vector<int> sol = g->searchDFS(i, thresh);
					for (auto x : sol) {
						visited[x] = true;
					}
					clusters.push_back(sol);
				}
			}
			delete[] visited;

			auto result = noe->noisy(clusters);

			res_clusters[i] = clusters;
			res[i] = result;
			std::cout << result << "\n";
			delete noe;
		}
	}
}

#endif // CL_H

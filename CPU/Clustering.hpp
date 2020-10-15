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
	void learn_parameters(Graph* g, std::tuple<double, double, double>* res, std::vector<std::vector<int>>* res_clusters);
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

	std::tuple<double, double, double>* res = new std::tuple<double, double, double>[portions + 1];
	std::vector<std::vector<int>>* res_clusters = new std::vector<std::vector<int>>[portions + 1];
	learn_parameters(g, res, res_clusters);

	for (int i = 0; i < lenRules; i++) {
		Rule& currRule = rules_adj_list[ind_ptr + i];
		if (currRule.isBuffered()) {
			currRule.removeBuffer();
		}
	}

	std::cout << "Calced params\n";

	int max_param = 0;
	double max_thresh = 0.0;
	double max_h1 = 0.0;
	double max_h3 = 0.0;
	double max_h10 = 0.0;

	for (int i = 0; i < portions + 1; i++) {

		double thresh = (double)i / portions;

		double hits1;
		double hits3;
		double hits10;

		std::tie(hits1, hits3, hits10) = res[i];

		if (hits1 > max_h1) {
			max_param = i;
			max_h1 = hits1;
			max_thresh = thresh;
		}
		else if (hits1 == max_h1 && hits3 > max_h3) {
			max_param = i;
			max_h3 = hits3;
			max_thresh = thresh;
		}
		else if (hits1 == max_h1 && hits3 == max_h3 && hits10 > max_h10) {
			max_param = i;
			max_h10 = hits10;
			max_thresh = thresh;
		}
	}

	std::cout << "MAX " << max_thresh << " " << max_h1 << " " << max_h3 << " " << max_h10 << "\n";
	std::ostringstream stringStream;
	stringStream << "Relation\t" << *index->getStringOfRelId(relation) << "\t" << max_thresh << "\n";
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

void Clustering::learn_parameters(Graph * g, std::tuple<double, double, double>* res, std::vector<std::vector<int>>* res_clusters) {
#pragma omp parallel for schedule(dynamic)
	for(int i = 0; i < portions+1; i++){
		double thresh = (double)i / portions;

		std::cout << thresh << "\n";

		if (thresh == 0.0) {
			std::vector<std::vector<int>> clusters;
			std::vector<int> cluster;
			for (int i = 0; i < samples_size; i++) {
				Rule& r = rules_adj_list[ind_ptr + i];
				if (r.is_c() || r.is_ac2()) {
					cluster.push_back(i);
				}
			}
			clusters.push_back(cluster);

			std::cout << clusters[0].size() << "\n";

			NoisyOrEngine* noe = new NoisyOrEngine(relation, index, graph, ttr, vtr, rr, clusters);
			auto result = noe->max();

			res[i] = result;
			res_clusters[i] = clusters;
			delete noe;
		} else {

			std::vector<std::vector<int>> clusters;

			bool* visited = new bool[samples_size];

			for (int i = 0; i < samples_size; i++) {
				visited[i] = false;
			}

			for (int i = 0; i < samples_size; i++) {
				Rule& r = rules_adj_list[ind_ptr + i];
				if (r.is_c() || r.is_ac2()) {
					if (visited[i] == false) {
						visited[i] = true;
						std::vector<int> sol = g->searchDFS(i, thresh);
						for (auto x : sol) {
							visited[x] = true;
						}
						clusters.push_back(sol);
					}
				}
			}
			delete[] visited;
			
			NoisyOrEngine* noe = new NoisyOrEngine(relation, index, graph, ttr, vtr, rr, clusters);
			auto result = noe->noisy();

			res_clusters[i] = clusters;
			res[i] = result;
			delete noe;
		}
	}
}

#endif // CL_H

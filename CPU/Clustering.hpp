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
	void addSolution(Rule& r, int ruleid, std::vector<std::vector<int>>& heads, std::vector<std::vector<int>>& tails);
	std::string learn_cluster();
private:
	int portions;
	uint32_t samples_size;
	uint32_t features_size;
	uint64_t total_size;
	int relation;
	std::vector<long long>* solutions;

	std::unordered_map<int, std::vector<int>>* c_head_solutions;
	std::unordered_map<int, std::vector<int>>* c_tail_solutions;
	std::unordered_set<int>* ac2;

	std::unordered_set<long long>* valid_ex;
	Rule* rules;

	int k = 200;

	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;
	RuleReader* rr;

	int WORKER_THREADS;

	static bool sortbysec(const std::pair<int, double>& a, const std::pair<int, double>& b)
	{
		return (a.second >= b.second);
	}

	void calc_jaccs(std::vector<std::pair<int, double>>* jacc);
	void learn_parameters(Graph* g, std::tuple<double, double, double>* res, std::vector<std::vector<int>>* res_clusters);
	MinHash* min;

	std::string cluster();


	double calc_jacc(std::vector<std::pair<int, int>>& this_vec, std::vector<std::pair<int, int>>& that_vec, bool jacc);
};

Clustering::Clustering(int portions, int relation, int size, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->portions = portions;
	this->index = index;
	this->graph = graph;
	this->ttr = ttr;
	this->vtr = vtr;
	this->rr = rr;
	this->relation = relation;
	samples_size = size;
	features_size = size;
	this->solutions = new std::vector<long long>[size];
	this->valid_ex = new std::unordered_set<long long>[size];

	this->rules = new Rule[size];
	WORKER_THREADS = Properties::get().WORKER_THREADS;
	min = new MinHash(k);
}


void Clustering::addSolution(Rule& r, int ruleid, std::vector<std::vector<int>>& heads, std::vector<std::vector<int>>& tails) {
	rules[ruleid] = r;
	if (heads.size() == 0) return;
	solutions[ruleid] = min->getMinimum(heads, tails);
}

std::string Clustering::learn_cluster() {
	return cluster();
}

void Clustering::calc_jaccs(std::vector<std::pair<int,double>>* jacc) {
#pragma omp parallel for schedule(dynamic)
	for(int i = 0; i < samples_size; i++){
		for (int j = 0; j < samples_size; j++) {
			if (i != j) {

				Rule& rule_i = rules[i];
				Rule& rule_j = rules[j];

				if (rule_i.is_c() and rule_j.is_c()) {
					int c = 0;
					for (int m = 0; m < k; m++) {
						if (solutions[i][m] == solutions[j][m]) {
							c++;
						}
					}
					double jaccard = (double)c / k;
					if (jaccard > 0.0) {
						jacc[i].push_back(std::make_pair(j, jaccard));
					}
				}
				else if (rule_i.is_ac2() and rule_j.is_ac2()) {
					int c = 0;
					for (int m = 0; m < k; m++) {
						if (solutions[i][m] == solutions[j][m]) {
							c++;
						}
					}
					double jaccard = (double)c / k;
					if (jaccard > 0.0) {
						jacc[i].push_back(std::make_pair(j, jaccard));
					}
				}
				else if ((rule_i.is_c() and rule_j.is_ac2()) or (rule_i.is_ac2() and rule_j.is_c())) {
					int c = 0;
					for (int m = 0; m < k; m++) {
						if (solutions[i][m] == solutions[j][m]) {
							c++;
						}
					}
					double jaccard = (double)c / k;
					if (jaccard > 0.0) {
						jacc[i].push_back(std::make_pair(j, jaccard));
					}
				}
			}
		}
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
				if (rules[i].is_c() || rules[i].is_ac2()) {
					cluster.push_back(i);
				}
			}
			clusters.push_back(cluster);

			std::cout << clusters[0].size() << "\n";

			NoisyOrEngine* noe = new NoisyOrEngine(relation, index, graph, ttr, vtr, rr, clusters);
			auto result = noe->max();

			res[i] = result;
			std::cout << "MAX" << "\n";
			std::cout << std::get<0>(result) << " " << std::get<1>(result) << " " << std::get<2>(result) << "\n";
			res_clusters[i] = clusters;
			delete noe;
		} else {

			std::vector<std::vector<int>> clusters;

			bool* visited = new bool[samples_size];

			for (int i = 0; i < samples_size; i++) {
				visited[i] = false;
			}

			for (int i = 0; i < samples_size; i++) {
				if (rules[i].is_c() || rules[i].is_ac2()) {
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

			std::cout << std::get<0>(result) << " " << std::get<1>(result) << " " << std::get<2>(result) << "\n";
			delete noe;
		}
	}
}

std::string Clustering::cluster() {
	delete min;
	std::vector<std::pair<int, double>>* jacc = new std::vector<std::pair<int, double>>[samples_size];
	//Foreach testtriples
	calc_jaccs(jacc);
	delete[] solutions;
	Graph* g = new Graph(samples_size, jacc, rules);
	std::cout << "Calced jaccs\n";

	std::tuple<double, double, double>* res = new std::tuple<double, double, double>[portions + 1];
	std::vector<std::vector<int>>* res_clusters = new std::vector<std::vector<int>>[portions + 1];
	learn_parameters(g, res, res_clusters);

	int* adj_begin = rr->getCSR()->getAdjBegin();
	Rule* rules_adj_list = rr->getCSR()->getAdjList();
	int ind_ptr = adj_begin[3 + relation];
	int lenRules = adj_begin[3 + relation + 1] - ind_ptr;
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
			Rule r = rules[rule];
			stringStream << r.getRulestring() << "\t";
		}
		stringStream << "\n";
	}
	stringStream << "\n";
	delete[] ac2;
	delete[] valid_ex;
	delete[] jacc;
	delete g;
	delete[] res;
	delete[] res_clusters;
	delete[] rules;

	return stringStream.str();
}

double Clustering::calc_jacc(std::vector<std::pair<int, int>> &this_vec, std::vector<std::pair<int, int>> &that_vec, bool jacc) {
	auto this_it = this_vec.begin();
	auto that_it = that_vec.begin();
	auto this_end = this_vec.end();
	auto that_end = that_vec.end();


	int in = 0;
	int that_un = 0;
	int this_un = 0;
	int un = 0;

	while (this_it != this_end || that_it != that_end) {



		if (this_it == this_end && that_it != that_end) {
			if (*that_it != *(that_it + 1) || that_it + 1 == that_end) {
				that_un++;
				un++;
			}
			that_it++;
		}
		else if (this_it != this_end && that_it == that_end) {
			if (*this_it != *(this_it + 1) || this_it + 1 == this_end) {
				this_un++;
				un++;
			}
			this_it++;
		}
		else {
			if (*this_it == *that_it) {
				if ((*this_it == *(this_it + 1) && this_it + 1 != this_end) && (*that_it == *(that_it + 1) && that_it + 1 != that_end)) {
					that_it++;
					this_it++;
				}
				else if (*this_it == *(this_it + 1) && this_it + 1 != this_end) {
					this_it++;
				}
				else if (*that_it == *(that_it + 1) && that_it + 1 != that_end) {
					that_it++;
				}
				else {
					in++;
					this_un++;
					that_un++;
					un++;
					this_it++;
					that_it++;
				}

			}
			else if (*this_it < *that_it) {
				if (*this_it != *(this_it + 1) || this_it + 1 == this_end) {
					this_un++;
					un++;
				}
				this_it++;
			}
			else {
				if (*that_it != *(that_it + 1) || that_it + 1 == that_end) {
					that_un++;
					un++;
				}
				that_it++;
			}
		}

	}
	double jaccard = 0.0;

	if (jacc) {
		if (un > 0) {
			return (double)in / (double)un;
		}
		else {
			return 0.0;
		}
	}
	else {
		if (this_un <= that_un) {
			if (this_un == 0) {
				return 0.0;
			}
			return (double)in / (double)this_un;
		}
		else {
			return (double)in / (double)that_un;
		}
	}
}

#endif // CL_H

#include "JaccardEngine.h"

JaccardEngine::JaccardEngine(Index* index, TraintripleReader* graph, ValidationtripleReader* vtr, RuleReader* rr) {
	this->index = index;
	this->graph = graph;
	this->vtr = vtr;
	this->rr = rr;
	this->k = Properties::get().RESOLUTION;
	adj_lists = graph->getCSR()->getAdjList();
	adj_list_starts = graph->getCSR()->getAdjBegin();
	vt_adj_lists = vtr->getCSR()->getAdjList();
	vt_adj_list_starts = vtr->getCSR()->getAdjBegin();
	rules_adj_list = rr->getCSR()->getAdjList();
	adj_begin = rr->getCSR()->getAdjBegin();
	WORKER_THREADS = Properties::get().WORKER_THREADS;
	this->rulegraph = new RuleGraph(index->getNodeSize(), graph);
	min = new MinHash(k);
}

void JaccardEngine::calculate_jaccard() {
	int rellen = index->getRelSize();
	for (int i = 0; i < rellen; i++) {

		int ind_ptr = adj_begin[3 + i];
		int len = adj_begin[3 + i + 1] - ind_ptr;
		std::cout << "Jaccard calculation for " << *(index->getStringOfRelId(i)) << " " << len << std::endl;
		Rule** rules = new Rule * [len];
		std::vector<long long>* solutions = new std::vector<long long>[len];
		std::cout << "Calulating all solutions of all rules ... \n";
		calc_sols(solutions, rules, ind_ptr, len);
		std::cout << "Calculating jaccards...\n";

		std::vector<std::pair<int, double>>* jacc = new std::vector<std::pair<int, double>>[len];
		calc_jaccs(solutions, rules, len, jacc);

		std::string out_path = Properties::get().PATH_JACCARD + std::string("/") + std::to_string(i) + std::string("_jacc.bin");
		std::ofstream file(out_path, std::ios::binary);


		file.write((char*)(&len), sizeof len);
		for (int i = 0; i < len; i++) {
			int n_jacc_gt0 = jacc[i].size();
			file.write((char*)(&n_jacc_gt0), sizeof n_jacc_gt0);
			for (int j = 0; j < n_jacc_gt0; j++) {
				int r = jacc[i][j].first;
				double jc = jacc[i][j].second;
				file.write((char*)(&r), sizeof r);
				file.write((char*)(&jc), sizeof jc);
			}
		}
		file.close();
		std::cout << "Jaccard calculated for relation " << i << "/" << rellen << std::endl;
		delete[] rules;
		delete[] solutions;
		delete[] jacc;
	}

	std::cout << "DONEZO" << std::endl;
}

void JaccardEngine::calc_sols(std::vector<long long>* solutions, Rule** rules, int ind_ptr, int len) {
	int size = index->getNodeSize();

#pragma omp parallel for schedule(dynamic)
	for (int j = 0; j < len; j++) {
		if (len > 100 && (j % ((len - 1) / 100)) == 0) {
			util::printProgress((double)j / (double)(len - 1));
		}

		std::vector<std::vector<int>> heads;
		std::vector<std::vector<int>> tails;

		Rule& currRule = rules_adj_list[ind_ptr + j];
		rules[j] = &currRule;

		int rulelength = currRule.getRulelength();
		bool** visited = new bool* [rulelength];
		for (int i = 0; i < rulelength; i++) {
			visited[i] = new bool[size];
			std::fill(visited[i], visited[i] + size, false);
		}
		int* previous = new int[rulelength];

		if (currRule.getRuletype() == Ruletype::XRule) {
			if (currRule.getBodyconstantId() != nullptr) {
				std::vector<int> results;
				rulegraph->searchDFSSingleStart(*currRule.getBodyconstantId(), currRule, true, results, previous, visited);
				heads.push_back(results);
				tails.push_back(std::vector<int> {*currRule.getHeadconstant()});
			}
			else {
				std::vector<int> results;
				rulegraph->searchDFSMultiStart(currRule, true, results);

				std::vector<int> filt_results;
				for (auto res : results) {
					if (currRule.head_exceptions.find(res) != currRule.head_exceptions.end()) {
						continue;
					}
					filt_results.push_back(res);
				}

				heads.push_back(filt_results);
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
				rulegraph->searchDFSMultiStart(currRule, true, results);
				heads.push_back(std::vector<int> {*currRule.getHeadconstant()});

				std::vector<int> filt_results;
				for (auto res : results) {
					if (currRule.tail_exceptions.find(res) != currRule.tail_exceptions.end()) {
						continue;
					}
					filt_results.push_back(res);
				}

				tails.push_back(filt_results);
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
					if (currRule.head_exceptions.find(val) != currRule.head_exceptions.end()) {
						continue;
					}
					std::vector<int> results;
					rulegraph->searchDFSSingleStart(val, currRule, false, results, previous, visited);
					if (results.size() > 0) {
						std::vector<int> filt_results;
						for (auto res : results) {
							if (currRule.tail_exceptions.find(res) != currRule.tail_exceptions.end()) {
								continue;
							}
							filt_results.push_back(res);
						}

						heads.push_back(std::vector<int> {val});
						tails.push_back(filt_results);
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
		solutions[j] = min->getMinimum(heads, tails);

		//std::cout << currRule.getRuletype() << " " << *headresultlength << " " << *tailresultlength << " " << currRule.getRulelength() << std::endl;


	}
}

void JaccardEngine::calc_jaccs(std::vector<long long>* solutions, Rule** rules, int len, std::vector<std::pair<int, double>>* jacc) {
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < len; i++) {
		if (len > 100 && (i % ((len - 1) / 100)) == 0) {
			util::printProgress((double)i / (double)(len - 1));
		}
		for (int j = 0; j < len; j++) {
			if (i != j) {
				/*
				Rule rule_i = rules[i];
				Rule rule_j = rules[j];

				if (rule_i.is_c() && rule_j.is_c()) {
					double jaccard = calc_jacc_samp(solutions[i], solutions[j], true);
					if (jaccard > 0.0) {
						jacc[i].push_back(std::make_pair(j, jaccard));
					}
				}
				else if (rule_i.is_ac2() && rule_j.is_ac2()) {
					double jaccard = calc_jacc_samp(solutions[i], solutions[j], true);
					if (jaccard > 0.0) {
						jacc[i].push_back(std::make_pair(j, jaccard));
					}
				}
				else if ((rule_i.is_c() && rule_j.is_ac2()) || (rule_i.is_ac2() && rule_j.is_c())) {
					double jaccard = calc_jacc_samp(solutions[i], solutions[j], true);
					if (jaccard > 0.0) {
						jacc[i].push_back(std::make_pair(j, jaccard));
					}
				}
				*/

				Rule& rule_i = *rules[i];
				Rule& rule_j = *rules[j];
				/*
				if (rule_i.get_body_hash() == rule_j.get_body_hash()) {
					jacc[i].push_back(std::make_pair(j, 1.0));
				}
				else {
				*/
				if ((rule_i.is_ac2() || rule_i.is_ac1()) && (rule_j.is_ac2() || rule_j.is_ac1()) && rule_i.getRuletype() == rule_j.getRuletype() && *rule_i.getHeadconstant() != *rule_j.getHeadconstant()) {
					continue;
				}
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
				//}
			}
		}
	}
}

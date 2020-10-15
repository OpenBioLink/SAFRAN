#ifndef RULEGRAPH_H
#define RULEGRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include "Rule.hpp"
#include "TraintripleReader.hpp"

class RuleGraph {
public:
	RuleGraph(int nodesize, TraintripleReader* graph) {
		this->size = nodesize;
		this->graph = graph;
		adj_lists = graph->getCSR()->getAdjList();
		adj_list_starts = graph->getCSR()->getAdjBegin();
	}

	RuleGraph(int nodesize, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr) {
		this->size = nodesize;
		this->graph = graph;
		adj_lists = graph->getCSR()->getAdjList();
		adj_list_starts = graph->getCSR()->getAdjBegin();
		this->train_relHeadToTails = graph->getrelHeadToTails();
		this->test_relHeadToTails = ttr->getRelHeadToTails();
		this->valid_relHeadToTails = vtr->getRelHeadToTails();
		this->relCounter = graph->getRelCounter();
	}

	void searchDFSSingleStart_filt(bool headNotTail, int filt_v, int v, Rule& r, bool bwd, std::vector<int>& solution, bool filtValidNotTest) {
	
		int rulelength = r.getRulelength();
		int* relations;
		if (bwd) {
			relations = r.getRelationsBwd();
		}
		else {
			relations = r.getRelationsFwd();
		}
		std::vector<int> previous(rulelength);
		std::vector<std::vector<bool>> visited(rulelength, std::vector<bool>(size));
		searchDFSUtil_filt(&r, headNotTail, filt_v, v, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND, filtValidNotTest);
	}

	void searchDFSSingleStart(int v, Rule& r, bool bwd, std::vector<int>& solution, int* previous, bool** visited) {
		int rulelength = r.getRulelength();
		int* relations;
		if (bwd) {
			relations = r.getRelationsBwd();
		}
		else {
			relations = r.getRelationsFwd();
		}
		searchDFSUtil(v, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND);
	}

	/*
	void searchDFSMultiStart(bool headNotTail, Rule& r, bool bwd, std::vector<int>& solution) {
		int rulelength = r.getRulelength();
		int* previous = new int[rulelength];
		int* relations;
		if (bwd) {
			relations = r.getRelationsBwd();
		}
		else {
			relations = r.getRelationsFwd();
		}
		bool** visited = new bool* [rulelength];
		for (int i = 0; i < rulelength; i++) {
			visited[i] = new bool[size];
			std::fill(visited[i], visited[i] + size, false);
		}

		int* adj_list = &(adj_lists[adj_list_starts[*relations]]);
		int start_indptr = 3;
		int size_indptr = adj_list[1];
		int start_ind = start_indptr + size_indptr;
		for (int val = 0; val < size_indptr - 1; val++) {
			int ind_ptr = adj_list[start_indptr + val];
			int len = adj_list[start_indptr + val + 1] - ind_ptr;
			if (len > 0) {
				searchDFSUtil(headNotTail, val, val, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND);
			}
		}

		for (int i = 0; i < rulelength; i++) {
			delete[] visited[i];
		}
		delete[] visited;
		delete[] previous;
	}
	*/
private:
	int size;
	TraintripleReader* graph;
	int* adj_lists;
	int* adj_list_starts;
	RelNodeToNodes train_relHeadToTails;
	RelNodeToNodes test_relHeadToTails;
	RelNodeToNodes valid_relHeadToTails;
	std::unordered_map<int, std::unordered_set<int>>* relCounter;

	void searchDFSUtil_filt(Rule* r, bool headNotTail, int filt_value, int value, std::vector<int>& solution, int* relations, std::vector<std::vector<bool>>& visited, std::vector<int>& previous, int level, int rulelength, int limit, bool filtValidNotTest) {

		if (solution.size() >= limit and limit > 0) {
			return;
		}
		if (level >= rulelength) {
			visited[level - 1][value] = true;
			int relation = *r->getHeadrelation();
			int head;
			int tail;
			if (headNotTail) {
				head = filt_value;
				tail = value;
			}
			else {
				head = value;
				tail = filt_value;
			}


			
			if (train_relHeadToTails.find(relation) != train_relHeadToTails.end()) {
				if (train_relHeadToTails[relation].find(head) != train_relHeadToTails[relation].end()) {
					if (train_relHeadToTails[relation][head].find(tail) != train_relHeadToTails[relation][head].end()) {
						return;
					}
				}
			}

			if (filtValidNotTest) {
				if (valid_relHeadToTails.find(relation) != valid_relHeadToTails.end()) {
					if (valid_relHeadToTails[relation].find(head) != valid_relHeadToTails[relation].end()) {
						if (valid_relHeadToTails[relation][head].find(tail) != valid_relHeadToTails[relation][head].end()) {
							return;
						}
					}
				}
			}
			else {
				if (test_relHeadToTails.find(relation) != test_relHeadToTails.end()) {
					if (test_relHeadToTails[relation].find(head) != test_relHeadToTails[relation].end()) {
						if (test_relHeadToTails[relation][head].find(tail) != test_relHeadToTails[relation][head].end()) {
							return;
						}
					}
				}
			}
			
			if (r->is_c()) {
				if (!headNotTail and r->head_exceptions.find(head) != r->head_exceptions.end()) {
					return;
				}
				if (headNotTail and r->tail_exceptions.find(tail) != r->tail_exceptions.end()) {
					return;
				}
			}

			if (Properties::get().ONLY_UNCONNECTED == 1) {
				if ((*relCounter).find(head) != (*relCounter).end()) {
					auto& it = (*relCounter).find(head)->second;
					if (it.find(tail) != it.end()) {
						return;
					}
				}
			}


			solution.push_back(value);
			return;
		}
		int this_level = level;
		previous[this_level] = value;
		int* adj_list = &(adj_lists[adj_list_starts[*relations]]);

		int start_indptr = 3;
		int size_indptr = adj_list[1];
		int start_ind = start_indptr + size_indptr;
		int ind_ptr = adj_list[start_indptr + value];
		int len = adj_list[start_indptr + value + 1] - ind_ptr;

		relations++;
		level++;

		bool node_fully_visited = true;
		for (int j = 0; j < len; j++)
		{
			int& nextval = adj_list[start_ind + ind_ptr + j];
			if (visited[this_level][nextval] == false) {
				bool inPrevious = false;
				for (int i = 0; i < level; i++) {
					if (previous[i] == nextval) {
						inPrevious = true;
					}
				}
				if (inPrevious) {
					node_fully_visited = false;
					continue;
				}
				searchDFSUtil_filt(r, headNotTail, filt_value, nextval, solution, relations, visited, previous, level, rulelength, limit, filtValidNotTest);
				if (solution.size() >= limit and limit > 0) {
					return;
				}
			}
		}
		if (node_fully_visited and this_level > 0) {
			visited[this_level - 1][value] = true;
		}
	}

	void searchDFSUtil(int value, std::vector<int>& solution, int* relations, bool** visited, int* previous, int level, int rulelength, int limit) {

		if (solution.size() >= limit and limit > 0) {
			return;
		}
		if (level >= rulelength) {
			visited[level - 1][value] = true;
			solution.push_back(value);
			return;
		}
		int this_level = level;
		previous[this_level] = value;
		int* adj_list = &(adj_lists[adj_list_starts[*relations]]);

		int start_indptr = 3;
		int size_indptr = adj_list[1];
		int start_ind = start_indptr + size_indptr;
		int ind_ptr = adj_list[start_indptr + value];
		int len = adj_list[start_indptr + value + 1] - ind_ptr;

		relations++;
		level++;

		bool node_fully_visited = true;
		for (int j = 0; j < len; j++)
		{
			int& nextval = adj_list[start_ind + ind_ptr + j];
			if (visited[this_level][nextval] == false) {
				bool inPrevious = false;
				for (int i = 0; i < level; i++) {
					if (previous[i] == nextval) {
						inPrevious = true;
					}
				}
				if (inPrevious) {
					node_fully_visited = false;
					continue;
				}
				searchDFSUtil(nextval, solution, relations, visited, previous, level, rulelength, limit);
				if (solution.size() >= limit and limit > 0) {
					return;
				}
			}
		}
		if (node_fully_visited and this_level > 0) {
			visited[this_level - 1][value] = true;
		}
	}

};

#endif //RULEGRAPH_H
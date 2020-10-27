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
		this->train_relHeadToTails = graph->getRelHeadToTails();
		this->test_relHeadToTails = ttr->getRelHeadToTails();
		this->valid_relHeadToTails = vtr->getRelHeadToTails();
		this->train_relTailToHeads = graph->getRelTailToHeads();
		this->test_relTailToHeads = ttr->getRelTailToHeads();
		this->valid_relTailToHeads = vtr->getRelTailToHeads();
		this->relCounter = graph->getRelCounter();
	}

	void searchDFSSingleStart_filt(bool headNotTail, int filt_v, int v, Rule& r, bool bwd, std::vector<int>& solution, bool filtValidNotTest, bool filtExceptions) {
	
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

		std::set<int> trains;
		if (headNotTail) {
			auto it = train_relHeadToTails.find(*r.getHeadrelation());
			if (it != train_relHeadToTails.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					trains = it_v->second;
				}
			}
		}
		else {
			auto it = train_relTailToHeads.find(*r.getHeadrelation());
			if (it != train_relTailToHeads.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					trains = it_v->second;
				}
			}
		}

		std::set<int> second_filt_set;
		if (filtValidNotTest) {
			if (headNotTail) {
				auto it = valid_relHeadToTails.find(*r.getHeadrelation());
				if (it != valid_relHeadToTails.end()) {
					auto it_v = it->second.find(filt_v);
					if (it_v != it->second.end()) {
						trains = it_v->second;
					}
				}
			}
			else {
				auto it = valid_relTailToHeads.find(*r.getHeadrelation());
				if (it != valid_relTailToHeads.end()) {
					auto it_v = it->second.find(filt_v);
					if (it_v != it->second.end()) {
						trains = it_v->second;
					}
				}
			}
		}
		else {
			if (headNotTail) {
				auto it = test_relHeadToTails.find(*r.getHeadrelation());
				if (it != test_relHeadToTails.end()) {
					auto it_v = it->second.find(filt_v);
					if (it_v != it->second.end()) {
						trains = it_v->second;
					}
				}
			}
			else {
				auto it = test_relTailToHeads.find(*r.getHeadrelation());
				if (it != test_relTailToHeads.end()) {
					auto it_v = it->second.find(filt_v);
					if (it_v != it->second.end()) {
						trains = it_v->second;
					}
				}
			}
		}

		searchDFSUtil_filt(&r, headNotTail, filt_v, filt_v, v, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND, filtValidNotTest, filtExceptions, trains, second_filt_set);
		std::sort(solution.begin(), solution.end());
	}

	void searchDFSMultiStart_filt(bool headNotTail, int filt_v, Rule& r, bool bwd, std::vector<int>& solution, bool filtValidNotTest, bool filtExceptions) {
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

		std::set<int> trains;
		if (headNotTail) {
			trains = train_relHeadToTails[*r.getHeadrelation()][filt_v];

		}
		else {
			trains = train_relTailToHeads[*r.getHeadrelation()][filt_v];
		}

		std::set<int> second_filt_set;
		if (filtValidNotTest) {
			if (headNotTail) {
				second_filt_set = valid_relHeadToTails[*r.getHeadrelation()][filt_v];

			}
			else {
				second_filt_set = valid_relTailToHeads[*r.getHeadrelation()][filt_v];
			}
		}
		else {
			if (headNotTail) {
				second_filt_set = test_relHeadToTails[*r.getHeadrelation()][filt_v];

			}
			else {
				second_filt_set = test_relTailToHeads[*r.getHeadrelation()][filt_v];
			}
		}

		int* adj_list = &(adj_lists[adj_list_starts[*relations]]);
		int start_indptr = 3;
		int size_indptr = adj_list[1];
		int start_ind = start_indptr + size_indptr;
		for (int val = 0; val < size_indptr - 1; val++) {
			//OI
			if (val == *r.getHeadconstant()) continue;
			int ind_ptr = adj_list[start_indptr + val];
			int len = adj_list[start_indptr + val + 1] - ind_ptr;
			if (len > 0) {
				searchDFSUtil_filt(&r, headNotTail, filt_v, val, val, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND, filtValidNotTest, filtExceptions, trains, second_filt_set);
			}
		}
		std::sort(solution.begin(), solution.end());
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
		std::sort(solution.begin(), solution.end());
	}

	void searchDFSMultiStart(Rule& r, bool bwd, std::vector<int>& solution) {
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
			//OI
			if (val == *r.getHeadconstant()) continue;
			if (r.getRuletype() == Ruletype::YRule and r.head_exceptions.find(val) != r.head_exceptions.end()) {
				continue;
			}
			if (r.getRuletype() == Ruletype::XRule and r.tail_exceptions.find(val) != r.tail_exceptions.end()) {
				continue;
			}
			int ind_ptr = adj_list[start_indptr + val];
			int len = adj_list[start_indptr + val + 1] - ind_ptr;
			if (len > 0) {
				searchDFSUtil(val, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND);
			}
		}

		for (int i = 0; i < rulelength; i++) {
			delete[] visited[i];
		}
		delete[] visited;
		delete[] previous;
		std::sort(solution.begin(), solution.end());
	}

private:
	int size;
	TraintripleReader* graph;
	int* adj_lists;
	int* adj_list_starts;
	RelNodeToNodes train_relHeadToTails;
	RelNodeToNodes test_relHeadToTails;
	RelNodeToNodes valid_relHeadToTails;
	RelNodeToNodes train_relTailToHeads;
	RelNodeToNodes test_relTailToHeads;
	RelNodeToNodes valid_relTailToHeads;
	std::unordered_map<int, std::unordered_set<int>>* relCounter;

	void searchDFSUtil_filt(Rule* r, bool headNotTail, int filt_value, int filt_ex, int value, std::vector<int>& solution, int* relations, std::vector<std::vector<bool>>& visited, std::vector<int>& previous, int level, int rulelength, int limit, bool filtValidNotTest, bool filtExceptions, std::set<int>& trains, std::set<int>& second_filt_set) {

		if (solution.size() >= limit and limit > 0) {
			return;
		}
		if (level >= rulelength) {
			visited[level - 1][value] = true;
			int relation = *r->getHeadrelation();
			int head;
			int ex_head;
			int tail;
			int ex_tail;
			if (headNotTail) {
				head = filt_value;
				ex_head = filt_ex;
				tail = value;
				ex_tail = value;
			}
			else {
				head = value;
				ex_head = value;
				tail = filt_value;
				ex_tail = filt_ex;
			}

			
			auto tit = trains.find(value);
			if (tit != trains.end()) {
				return;
			}

			auto sfit = second_filt_set.find(value);
			if (sfit != second_filt_set.end()) {
				return;
			}
			
			if (filtExceptions and (r->is_c() or r->is_ac1())) {
				if (!headNotTail and r->head_exceptions.find(ex_head) != r->head_exceptions.end()) {
					return;
				}
				if (headNotTail and r->tail_exceptions.find(ex_tail) != r->tail_exceptions.end()) {
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
				searchDFSUtil_filt(r, headNotTail, filt_value, filt_ex, nextval, solution, relations, visited, previous, level, rulelength, limit, filtValidNotTest, filtExceptions, trains, second_filt_set);
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
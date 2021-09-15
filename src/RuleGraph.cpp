#include "RuleGraph.h"

RuleGraph::RuleGraph(int nodesize, TraintripleReader* graph) {
	this->size = nodesize;
	this->graph = graph;
	adj_lists = graph->getCSR()->getAdjList();
	adj_list_starts = graph->getCSR()->getAdjBegin();
}

RuleGraph::RuleGraph(int nodesize, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr) {
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

void RuleGraph::searchDFSSingleStart_filt(bool headNotTail, int filt_v, int v, Rule& r, bool bwd, std::vector<int>& solution, bool filtValidNotTest, bool filtExceptions) {
	
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

	std::set<int>* trains = nullptr;
	if (headNotTail) {
		auto it = train_relHeadToTails.find(*r.getHeadrelation());
		if (it != train_relHeadToTails.end()) {
			auto it_v = it->second.find(filt_v);
			if (it_v != it->second.end()) {
				trains = &it_v->second;
			}
		}
	}
	else {
		auto it = train_relTailToHeads.find(*r.getHeadrelation());
		if (it != train_relTailToHeads.end()) {
			auto it_v = it->second.find(filt_v);
			if (it_v != it->second.end()) {
				trains = &it_v->second;
			}
		}
	}

	std::set<int>* second_filt_set = nullptr;
	if (filtValidNotTest) {
		if (headNotTail) {
			auto it = valid_relHeadToTails.find(*r.getHeadrelation());
			if (it != valid_relHeadToTails.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					second_filt_set = &it_v->second;
				}
			}
		}
		else {
			auto it = valid_relTailToHeads.find(*r.getHeadrelation());
			if (it != valid_relTailToHeads.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					second_filt_set = &it_v->second;
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
					second_filt_set = &it_v->second;
				}
			}
		}
		else {
			auto it = test_relTailToHeads.find(*r.getHeadrelation());
			if (it != test_relTailToHeads.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					second_filt_set = &it_v->second;
				}
			}
		}
	}
	int* c = new int(0);
	searchDFSUtil_filt(&r, headNotTail, filt_v, filt_v, v, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND, c, filtValidNotTest, filtExceptions, trains, second_filt_set);
	delete c;
	std::sort(solution.begin(), solution.end());
}

void RuleGraph::searchDFSMultiStart_filt(bool headNotTail, int filt_v, Rule& r, bool bwd, std::vector<int>& solution, bool filtValidNotTest, bool filtExceptions) {
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

	std::set<int>* trains = nullptr;
	if (headNotTail) {
		auto it = train_relHeadToTails.find(*r.getHeadrelation());
		if (it != train_relHeadToTails.end()) {
			auto it_v = it->second.find(filt_v);
			if (it_v != it->second.end()) {
				trains = &it_v->second;
			}
		}
	}
	else {
		auto it = train_relTailToHeads.find(*r.getHeadrelation());
		if (it != train_relTailToHeads.end()) {
			auto it_v = it->second.find(filt_v);
			if (it_v != it->second.end()) {
				trains = &it_v->second;
			}
		}
	}

	std::set<int>* second_filt_set = nullptr;
	if (filtValidNotTest) {
		if (headNotTail) {
			auto it = valid_relHeadToTails.find(*r.getHeadrelation());
			if (it != valid_relHeadToTails.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					second_filt_set = &it_v->second;
				}
			}
		}
		else {
			auto it = valid_relTailToHeads.find(*r.getHeadrelation());
			if (it != valid_relTailToHeads.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					second_filt_set = &it_v->second;
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
					second_filt_set = &it_v->second;
				}
			}
		}
		else {
			auto it = test_relTailToHeads.find(*r.getHeadrelation());
			if (it != test_relTailToHeads.end()) {
				auto it_v = it->second.find(filt_v);
				if (it_v != it->second.end()) {
					second_filt_set = &it_v->second;
				}
			}
		}
	}

	int* adj_list = &(adj_lists[adj_list_starts[*relations]]);
	int start_indptr = 3;
	int size_indptr = adj_list[1];
	int start_ind = start_indptr + size_indptr;
	int* c = new int(0);
	for (int val = 0; val < size_indptr - 1; val++) {
		//OI
		if (val == *r.getHeadconstant()) continue;
		if (r.getRuletype() == Ruletype::YRule && r.head_exceptions.find(val) != r.head_exceptions.end()) {
			continue;
		}
		if (r.getRuletype() == Ruletype::XRule && r.tail_exceptions.find(val) != r.tail_exceptions.end()) {
			continue;
		}
		int ind_ptr = adj_list[start_indptr + val];
		int len = adj_list[start_indptr + val + 1] - ind_ptr;
		if (len > 0) {
			searchDFSUtil_filt(&r, headNotTail, filt_v, val, val, solution, relations, visited, previous, 0, rulelength, Properties::get().DISCRIMINATION_BOUND, c, filtValidNotTest, filtExceptions, trains, second_filt_set);
		}
	}
	delete c;
	std::sort(solution.begin(), solution.end());
}

bool RuleGraph::existsAcyclic(int* valId, Rule& rule, bool filtValidNotTest) {
	int* relations = rule.getRelationsFwd();
	int* constantnode = nullptr;
	if (rule.getBodyconstantId() != nullptr) {
		constantnode = rule.getBodyconstantId();
	}
	if (Properties::get().ONLY_UNCONNECTED == 1) {
		if ((*relCounter).find(*valId) != (*relCounter).end()) {
			auto& it = (*relCounter).find(*valId)->second;
			if (it.find(*rule.getHeadconstant()) != it.end()) {
				return false;
			}
		}
	}
	if (existsAcyclic(valId, constantnode, relations, rule.getRulelength())) {
		if (rule.getRuletype() == Ruletype::XRule) {
			auto it = train_relHeadToTails.find(*rule.getHeadrelation());
			if (it != train_relHeadToTails.end()) {
				auto it_v = it->second.find(*valId);
				if (it_v != it->second.end()) {
					if (it_v->second.find(*rule.getHeadconstant()) != it_v->second.end()) {
						return false;
					}
				}
			}
		}
		else {
			auto it = train_relTailToHeads.find(*rule.getHeadrelation());
			if (it != train_relTailToHeads.end()) {
				auto it_v = it->second.find(*valId);
				if (it_v != it->second.end()) {
					if (it_v->second.find(*rule.getHeadconstant()) != it_v->second.end()) {
						return false;
					}
				}
			}
		}
		if (filtValidNotTest) {
			if (rule.getRuletype() == Ruletype::XRule) {
				auto it = valid_relHeadToTails.find(*rule.getHeadrelation());
				if (it != valid_relHeadToTails.end()) {
					auto it_v = it->second.find(*valId);
					if (it_v != it->second.end()) {
						if (it_v->second.find(*rule.getHeadconstant()) != it_v->second.end()) {
							return false;
						}
					}
				}
			}
			else {
				auto it = valid_relTailToHeads.find(*rule.getHeadrelation());
				if (it != valid_relTailToHeads.end()) {
					auto it_v = it->second.find(*valId);
					if (it_v != it->second.end()) {
						if (it_v->second.find(*rule.getHeadconstant()) != it_v->second.end()) {
							return false;
						}
					}
				}
			}
		}
		else {
			if (rule.getRuletype() == Ruletype::XRule) {
				auto it = test_relHeadToTails.find(*rule.getHeadrelation());
				if (it != test_relHeadToTails.end()) {
					auto it_v = it->second.find(*valId);
					if (it_v != it->second.end()) {
						if (it_v->second.find(*rule.getHeadconstant()) != it_v->second.end()) {
							return false;
						}
					}
				}
			}
			else {
				auto it = test_relTailToHeads.find(*rule.getHeadrelation());
				if (it != test_relTailToHeads.end()) {
					auto it_v = it->second.find(*valId);
					if (it_v != it->second.end()) {
						if (it_v->second.find(*rule.getHeadconstant()) != it_v->second.end()) {
							return false;
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

void RuleGraph::searchDFSSingleStart(int v, Rule& r, bool bwd, std::vector<int>& solution, int* previous, bool** visited) {
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

void RuleGraph::searchDFSMultiStart(Rule& r, bool bwd, std::vector<int>& solution) {
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
		if (r.getRuletype() == Ruletype::YRule && r.head_exceptions.find(val) != r.head_exceptions.end()) {
			continue;
		}
		if (r.getRuletype() == Ruletype::XRule && r.tail_exceptions.find(val) != r.tail_exceptions.end()) {
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

void RuleGraph::searchDFSUtil_filt(Rule* r, bool headNotTail, int filt_value, int filt_ex, int value, std::vector<int>& solution, int* relations, std::vector<std::vector<bool>>& visited, std::vector<int>& previous, int level, int rulelength, int limit, int* c, bool filtValidNotTest, bool filtExceptions, std::set<int>* trains, std::set<int>* second_filt_set) {
	(*c)++;
	if (*c >= limit && limit > 0) {
		solution.clear();
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

		if (trains != nullptr) {
			auto tit = trains->find(value);
			if (tit != trains->end()) {
				return;
			}
		}
		if (second_filt_set != nullptr) {
			auto sfit = second_filt_set->find(value);
			if (sfit != second_filt_set->end()) {
				return;
			}
		}
			
		if (filtExceptions && (r->is_c() || r->is_ac1())) {
			if (!headNotTail && r->head_exceptions.find(ex_head) != r->head_exceptions.end()) {
				return;
			}
			if (headNotTail && r->tail_exceptions.find(ex_tail) != r->tail_exceptions.end()) {
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
			searchDFSUtil_filt(r, headNotTail, filt_value, filt_ex, nextval, solution, relations, visited, previous, level, rulelength, limit, c, filtValidNotTest, filtExceptions, trains, second_filt_set);
			if (*c >= limit && limit > 0) {
				return;
			}
		}
	}
	if (node_fully_visited && this_level > 0) {
		visited[this_level - 1][value] = true;
	}
}

void RuleGraph::searchDFSUtil(int value, std::vector<int>& solution, int* relations, bool** visited, int* previous, int level, int rulelength, int limit) {

	if (solution.size() >= limit && limit > 0) {
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
			if (solution.size() >= limit && limit > 0) {
				return;
			}
		}
	}
	if (node_fully_visited && this_level > 0) {
		visited[this_level - 1][value] = true;
	}
}

bool RuleGraph::existsAcyclic(int* valId, int* constant, int* relations, int N) {
	int* adj_list = &(adj_lists[adj_list_starts[*relations]]);
	int start_indptr = 3;
	int size_indptr = adj_list[1];
	int start_ind = start_indptr + size_indptr;
	//int size_ind = adj_list[2];

	relations++;
	N--;
	int ind_ptr = adj_list[start_indptr + *valId];
	int len = adj_list[start_indptr + *valId + 1] - ind_ptr;
	if (N == 0 && constant != nullptr) {
		if (util::in_sorted(&adj_list[start_ind + ind_ptr], len, *constant)) {
			return true;
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
			if (existsAcyclic(&adj_list[start_ind + ind_ptr + j], constant, relations, N)) {
				return true;
			};
		}
		return false;
	}
}

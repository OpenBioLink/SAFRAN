#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

#include "Rule.h"


class Graph {
public:
	Graph(int size, std::vector<std::pair<int, double>>* jacc, Rule* rules, int ind_ptr);
	std::vector<int> searchDFS(int v, double c_c, double ac1_ac1, double ac2_ac2, double c_ac2, double c_ac1, double ac1_ac2);

private:
	int* size;
	std::vector<std::pair<int, double>>* jacc;
	Rule* rules;
	int ind_ptr;
	void searchDFSUtil(std::vector<int>& solution, bool* visited, int v, double c_c, double ac1_ac1, double ac2_ac2, double c_ac2, double c_ac1, double ac1_ac2);

};

#endif //GRAPH_H
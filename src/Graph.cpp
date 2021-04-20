#include "Graph.h"

Graph::Graph(int size, std::vector<std::pair<int, double>>* jacc, Rule* rules, int ind_ptr) {
	this->size = new int(size);
	this->jacc = jacc;
	this->rules = rules;
	this->ind_ptr = ind_ptr;
}

std::vector<int> Graph::searchDFS(int v, double c_c, double ac1_ac1, double ac2_ac2, double c_ac2, double c_ac1, double ac1_ac2) {
	std::vector<int> solution;
	bool* visited = new bool[*size];
	std::fill(visited, visited + *size, false);

	searchDFSUtil(solution, visited, v, c_c, ac1_ac1, ac2_ac2, c_ac2, c_ac1, ac1_ac2);
	delete[] visited;
	return solution;
}


void Graph::searchDFSUtil(std::vector<int>& solution, bool* visited, int v, double c_c, double ac1_ac1, double ac2_ac2, double c_ac2, double c_ac1, double ac1_ac2) {

	std::stack<int> stack;

	// Push the current source node. 
	stack.push(v);

	while (!stack.empty()) {
		// Pop a vertex from stack && print it 
		v = stack.top();
		stack.pop();

		if (visited[v])
			continue;

		// Stack may contain same vertex twice. So 
		// we need to print the popped item only 
		// if it is not visited. 
		if (!visited[v])
		{
				
			solution.push_back(v);
			visited[v] = true;
		}

		// Get all adjacent vertices of the popped vertex s 
		// If a adjacent has not been visited, then push it 
		// to the stack. 
		for (auto tup : jacc[v]) {
			int u = tup.first;
			double conf = tup.second;
			if (!visited[u]) {
				Rule& rule_i = rules[ind_ptr + u];
				Rule& rule_j = rules[ind_ptr + v];

				if (rule_i.is_c() && rule_j.is_c() && conf > c_c) {
					stack.push(u);
				}
				else if (rule_i.is_ac1() && rule_j.is_ac1() && conf > ac1_ac1) {
					stack.push(u);
				}
				else if (rule_i.is_ac2() && rule_j.is_ac2() && conf > ac2_ac2) {
					stack.push(u);
				}
				else if (((rule_i.is_c() && rule_j.is_ac2()) || (rule_i.is_ac2() && rule_j.is_c())) && conf > c_ac2) {
					stack.push(u);
				}
				else if (((rule_i.is_c() && rule_j.is_ac1()) || (rule_i.is_ac1() && rule_j.is_c())) && conf > c_ac1) {
					stack.push(u);
				}
				else if (((rule_i.is_ac1() && rule_j.is_ac2()) || (rule_i.is_ac2() && rule_j.is_ac1())) && conf > ac1_ac2) {
					stack.push(u);
				}
			}
					
		}
	}
}
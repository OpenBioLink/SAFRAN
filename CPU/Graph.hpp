#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include "Rule.hpp"


class Graph {
public:
	Graph(int size, std::vector<std::pair<int, double>>* jacc, Rule* rules) {
		this->size = new int(size);
		this->jacc = jacc;
		this->rules = rules;
	}

	std::vector<int> searchDFS(int v, double thresh) {
		std::vector<int> solution;
		bool* visited = new bool[*size];
		std::fill(visited, visited + *size, false);

		searchDFSUtil(solution, visited, v, thresh);
		delete[] visited;
		return solution;
	}

private:
	int* size;
	std::vector<std::pair<int, double>>* jacc;
	Rule* rules;


	void searchDFSUtil(std::vector<int>& solution, bool* visited, int v, double thresh) {



		std::stack<int> stack;

		// Push the current source node. 
		stack.push(v);

		while (!stack.empty()) {
			// Pop a vertex from stack and print it 
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
					Rule& rule_i = rules[u];
					Rule& rule_j = rules[v];

					if (rule_i.is_c() && rule_j.is_c() && conf > thresh) {
						stack.push(u);
					}
					else if (rule_i.is_ac2() and rule_j.is_ac2() && conf > thresh) {
						stack.push(u);
					}
					else if (((rule_i.is_c() and rule_j.is_ac2()) or (rule_i.is_ac2() and rule_j.is_c())) && conf > thresh) {
						stack.push(u);
					}
				}
					
			}
		}
	}

};

#endif //GRAPH_H
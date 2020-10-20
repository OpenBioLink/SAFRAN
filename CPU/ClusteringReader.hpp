#ifndef CLUSTERINGREADER_H
#define CLUSTERINGREADER_H

#include <string>
#include <iostream>
#include <fstream>

#include "TraintripleReader.hpp"
#include "CSR.hpp"
#include "Types.h"
#include "Util.hpp"
#include "Rule.hpp"
#include "CSR.hpp"


class ClusteringReader
{
public:
	ClusteringReader(std::string clusteringpath, CSR<int, Rule>* rulecsr, Index* index, TraintripleReader* graph) {
		this->graph = graph;
		this->index = index;
		read(rulecsr, clusteringpath);
	}

	CSR<int, Rule>* getCSR() {
		return csr;
	}

	std::unordered_map<int, std::pair<double, std::vector<std::vector<int>>>>& getRelToClusters() {
		return relToClusters;
	}

protected:

private:
	TraintripleReader* graph;
	Index* index;
	CSR<int, Rule>* csr;
	std::unordered_map<int, std::pair<double, std::vector<std::vector<int>>>> relToClusters;

	void read(CSR<int, Rule>* rulecsr, std::string clusteringpath) {
		std::unordered_map<std::string, int> rulestringToID;

		int rellen = index->getRelSize();
		Rule* rules_adj_list = rulecsr->getAdjList();
		int* adj_begin = rulecsr->getAdjBegin();
		for (int i = 0; i < rellen; i++) {
			int ind_ptr = adj_begin[3 + i];
			int len = adj_begin[3 + i + 1] - ind_ptr;
			for (int j = 0; j < len; j++) {
				Rule currRule = rules_adj_list[ind_ptr + j];
				rulestringToID[currRule.getRulestring()] = j;
			}
		}

		std::string line;
		std::ifstream myfile(clusteringpath);
		if (myfile.is_open()) {
			int relation;
			while (getline(myfile, line))
			{
				while (line.compare("") == 0) {
					getline(myfile, line);
				}

				std::vector<std::string> rel_conf = util::split(line, '\t');
				int relation = *index->getIdOfRelationstring(rel_conf[1]);
				double thresh = std::stod(rel_conf[2]);


				std::vector<std::vector<int>> id_clusters;
				while (true) {
					getline(myfile, line);
					if (line.compare("") == 0) {
						break;
					}
					std::vector<std::string> rules = util::split(line, '\t');
					std::vector<int> id_cluster;
					for (auto rule : rules) {
						if (rule.compare("") == 0) {
							continue;
						}

						auto id = rulestringToID.find(rule);
						if (id != rulestringToID.end()) {
							id_cluster.push_back(id->second);
						}
						else {
							std::cout << "Rule in cluster not found in ruleset";
							exit(-1);
						}
					}
					id_clusters.push_back(id_cluster);
				}
				relToClusters[relation] = std::make_pair(thresh, id_clusters);
			}
			myfile.close();
		}
		else {
			std::cout << "Unable to open rule file " << clusteringpath << std::endl;
			exit(-1);
		}

	}
};

#endif // CLUSTERINGREADER_H

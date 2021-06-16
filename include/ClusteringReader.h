#ifndef CLUSTERINGREADER_H
#define CLUSTERINGREADER_H

#include <string>
#include <iostream>
#include <fstream>

#include "Index.h"

#include "CSR.hpp"
#include "Rule.h"
#include "TraintripleReader.h"
#include "Util.hpp"

class ClusteringReader
{
public:
	ClusteringReader(std::string clusteringpath, CSR<int, Rule>* rulecsr, Index* index, TraintripleReader* graph);
	CSR<int, Rule>* getCSR();
	std::unordered_map<int, std::pair<std::pair<bool, std::vector<std::vector<int>>>, std::pair<bool, std::vector<std::vector<int>>>>>& getRelToClusters();

protected:

private:
	TraintripleReader* graph;
	Index* index;
	CSR<int, Rule>* csr;
	std::unordered_map<int, std::pair<std::pair<bool, std::vector<std::vector<int>>>, std::pair<bool, std::vector<std::vector<int>>>>> relToClusters;

	std::unordered_map<std::string, int> rulestringToID;
	std::pair<bool, std::vector<std::vector<int>>> parseCluster(std::istream& myfile, std::string line, int* adj_begin, bool headNotTail);

	void read(CSR<int, Rule>* rulecsr, std::string clusteringpath);
};

#endif // CLUSTERINGREADER_H

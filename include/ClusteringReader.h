#ifndef CLUSTERINGREADER_H
#define CLUSTERINGREADER_H

#include <string>
#include <iostream>
#include <fstream>

#include "Index.h"

#include "CSR.h"
#include "Rule.h"
#include "TraintripleReader.h"
#include "Util.hpp"

class ClusteringReader
{
public:
	ClusteringReader(std::string clusteringpath, CSR<int, Rule>* rulecsr, Index* index, TraintripleReader* graph);
	CSR<int, Rule>* getCSR();
	std::unordered_map<int, std::pair<bool, std::vector<std::vector<int>>>>& getRelToClusters();

protected:

private:
	TraintripleReader* graph;
	Index* index;
	CSR<int, Rule>* csr;
	std::unordered_map<int, std::pair<bool, std::vector<std::vector<int>>>> relToClusters;

	void read(CSR<int, Rule>* rulecsr, std::string clusteringpath);
};

#endif // CLUSTERINGREADER_H

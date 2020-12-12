#ifndef RULEREADER_H
#define RULEREADER_H

#include <string>
#include <iostream>
#include <fstream>

#include <omp.h>
#include <regex>

#include "Index.h"
#include "Util.hpp"
#include "TraintripleReader.h"
#include "CSR.hpp"
#include "Types.h"

class RuleReader
{
public:
	RuleReader(std::string filepath, Index* index, TraintripleReader* graph);
	CSR<int, Rule>* getCSR();

protected:

private:
	TraintripleReader* graph;
	Index* index;
	CSR<int, Rule>* csr;

	void read(std::string filepath);
	Rule* parseRule(std::vector<std::string> rule);
	bool parseXtoY(Ruletype type, std::string& head, std::string& tail);
	std::string getRelation(std::string atom, std::string previous, int* relation);
	std::pair<std::string, std::string> getHeadTail(std::string& atom);
};

#endif // RULEREADER_H

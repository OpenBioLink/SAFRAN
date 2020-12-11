#ifndef TRAINTRIPLEREADER_H
#define TRAINTRIPLEREADER_H

#include <string>
#include <fstream>
#include <unordered_set>

#include "CSR.hpp"
#include "Index.h"
#include "Types.h"
#include "Util.hpp"

class TraintripleReader
{
public:
	TraintripleReader(std::string filepath, Index* index);

	CSR<int, int>* getCSR();
	std::unordered_map<int, std::unordered_set<int>>* getRelCounter();
	RelNodeToNodes& getRelHeadToTails();
	RelNodeToNodes& getRelTailToHeads();

protected:

private:
	Index* index;
	CSR<int, int>* csr;
	std::unordered_map<int, std::unordered_set<int>>* relCounter;
	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;

	void read(std::string filepath, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads, std::unordered_map<int, std::unordered_set<int>>* relCounter);
	void add(std::string head, std::string relation, std::string tail, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads, std::unordered_map<int, std::unordered_set<int>>* relCounter);


};

#endif // TRAINTRIPLEREADER_H

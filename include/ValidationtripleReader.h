#ifndef VALIDATIONTRIPLEREADER_H
#define VALIDATIONTRIPLEREADER_H

#include <string>
#include <vector>

#include "Index.h"
#include "TraintripleReader.h"

class ValidationtripleReader
{

public:
	ValidationtripleReader(std::string filepath, Index* index, TraintripleReader* graph);

	CSR<int, int>* getCSR();

	RelNodeToNodes& getRelHeadToTails();

	RelNodeToNodes& getRelTailToHeads();

	std::unordered_map<int, std::vector<std::pair<int, int>>> RelTuples;
protected:

private:

	Index* index;
	TraintripleReader* graph;
	CSR<int, int>* csr;

	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;

	void read(std::string filepath);
};

#endif //VALIDATIONTRIPLEREADER_H


#ifndef TESTTRIPLEREADER_H
#define TESTTRIPLEREADER_H

#ifdef __unix
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "Index.h"
#include "TraintripleReader.h"
#include "Util.hpp"
#include "Trial.h"
#include "Properties.h"

class TesttripleReader
{

public:
	TesttripleReader(std::string filepath, Index* index, TraintripleReader* graph);

	int** getTesttriples();
	int* getTesttriplesSize();
	CSR<int, int>* getCSR();
	std::vector<int>& getUniqueRelations();
	RelNodeToNodes& getRelHeadToTails();
	RelNodeToNodes& getRelTailToHeads();

protected:

private:
	int** testtriples;
	int* testtripleSize;
	Index* index;
	TraintripleReader* graph;
	std::vector<int> uniqueRelations;
	CSR<int, int>* csr;

	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;

	void read(std::string filepath);
};

#endif // TESTTRIPLEREADER_H

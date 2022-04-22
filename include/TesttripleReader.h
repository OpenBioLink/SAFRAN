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
#include "Properties.hpp"

class TesttripleReader
{

public:
    TesttripleReader(Index* index, TraintripleReader* graph, int is_trial);

	int** getTesttriples();
	int* getTesttriplesSize();
	CSR<int, int>* getCSR();
	RelNodeToNodes& getRelHeadToTails();
	RelNodeToNodes& getRelTailToHeads();

	void read(std::string filepath);
    void read(std::vector<std::tuple<std::string, std::string, std::string>> & triples);

protected:

private:
	int is_trial;
	int** testtriples;
	int* testtripleSize;
	Index* index;
	TraintripleReader* graph;
	CSR<int, int>* csr;

	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;
};

#endif // TESTTRIPLEREADER_H

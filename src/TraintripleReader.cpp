#include "TraintripleReader.h"

TraintripleReader::TraintripleReader(std::string filepath, Index * index) {
	this->index = index;
	relCounter = new std::unordered_map<int, std::unordered_set<int>>;

	read(filepath, relHeadToTails, relTailToHeads, relCounter);
			
	index->rehash();
	for (int i = 0; i < index->getRelSize(); i++) {
		relHeadToTails[i].rehash(relHeadToTails[i].size());
		relTailToHeads[i].rehash(relTailToHeads[i].size());
	}
	relHeadToTails.rehash(relHeadToTails.size());
	relTailToHeads.rehash(relTailToHeads.size());

	csr = new CSR<int, int>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
}

CSR<int, int>* TraintripleReader::getCSR() {
	return csr;
}

std::unordered_map<int, std::unordered_set<int>>* TraintripleReader::getRelCounter() {
	return relCounter;
}

RelNodeToNodes& TraintripleReader::getRelHeadToTails() {
	return relHeadToTails;
}

RelNodeToNodes& TraintripleReader::getRelTailToHeads() {
	return relTailToHeads;
}

void TraintripleReader::read(std::string filepath, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads, std::unordered_map<int, std::unordered_set<int>>* relCounter) {
	std::string line;
	std::ifstream myfile(filepath);
	if (myfile.is_open())
	{
		while (!util::safeGetline(myfile, line).eof())
		{
			std::istringstream iss(line);
			std::vector<std::string> results = util::split(line, '\t');
			if (results.size() != 3) {
				std::cout << "Unsupported Filetype, please make sure you have the following triple format {subject}{TAB}{predicate}{TAB}{object}" << std::endl;
				//exit(-1);
			}
			add(results[0], results[1], results[2], relHeadToTails, relTailToHeads, relCounter);
		}
		myfile.close();
	}
	else {
		std::cout << "Unable to open train file " << filepath << std::endl;
		exit(-1);
	}
}

void TraintripleReader::add(std::string head, std::string relation, std::string tail, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads, std::unordered_map<int, std::unordered_set<int>>* relCounter) {
	//Get ids
	index->addNode(head);
	index->addNode(tail);
	index->addRelation(relation);

	int headNodeId = *(index->getIdOfNodestring(head));
	int relId = *(index->getIdOfRelationstring(relation));
	int tailNodeId = *(index->getIdOfNodestring(tail));

	relHeadToTails[relId][headNodeId].insert(tailNodeId);
	relTailToHeads[relId][tailNodeId].insert(headNodeId);
	(*relCounter)[headNodeId].insert(tailNodeId);
	(*relCounter)[tailNodeId].insert(headNodeId);
}

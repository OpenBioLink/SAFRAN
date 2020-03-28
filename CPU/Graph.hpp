#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <fstream>

#include "Types.h"
#include "Index.hpp"
#include "Util.hpp"
#include "CSR.hpp"

class Graph
{
    public:
		Graph(std::string filepath, Index * index) {
			this->index = index;
			RelNodeToNodes relHeadToTails;
			RelNodeToNodes relTailToHeads;
			read(filepath, relHeadToTails, relTailToHeads);
			
			index->rehash();
			for (int i = 0; i < index->getRelSize(); i++) {
				relHeadToTails[i].rehash(relHeadToTails[i].size());
				relTailToHeads[i].rehash(relTailToHeads[i].size());
			}
			relHeadToTails.rehash(relHeadToTails.size());
			relTailToHeads.rehash(relTailToHeads.size());

			csr = new CSR<int, int>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
		}

		CSR<int, int>* getCSR() {
			return csr;
		}

    protected:

    private:
		Index * index;
		CSR<int, int> * csr;

		void read(std::string filepath, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads) {
			std::string line;
			std::ifstream myfile(filepath);
			if (myfile.is_open())
			{
				while (getline(myfile, line))
				{
					std::istringstream iss(line);
					std::vector<std::string> results = util::split(line, '\t');
					if (results.size() != 3) {
						throw "Unsupported Filetype";
					}
					add(results[0], results[1], results[2], relHeadToTails, relTailToHeads);
				}
				myfile.close();
			}
			else throw "Unable to open file";
		}

		void add(std::string head, std::string relation, std::string tail, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads) {
			//Get ids
			index->addNode(head);
			index->addNode(tail);
			index->addRelation(relation);

			int headNodeId = *(index->getIdOfNodestring(head));
			int relId = *(index->getIdOfRelationstring(relation));
			int tailNodeId = *(index->getIdOfNodestring(tail));

			relHeadToTails[relId][headNodeId].insert(tailNodeId);
			relTailToHeads[relId][tailNodeId].insert(headNodeId);
		}

		
};

#endif // GRAPH_H

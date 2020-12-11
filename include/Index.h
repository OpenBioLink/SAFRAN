#ifndef INDEX_H
#define INDEX_H

#include <unordered_map>
#include <iostream>

class Index {

public:
	void addNode(std::string& nodesstring);
	void addRelation(std::string& relstring);
	int getNodeSize();
	int getRelSize();
	int* getIdOfNodestring(std::string& node);
	std::string* getStringOfNodeId(int& id);
	int* getIdOfRelationstring(std::string& relation);
	std::string* getStringOfRelId(int& id);
	void rehash();

private:
	std::unordered_map<std::string, int> nodeToId;
	std::unordered_map<std::string, int> relToId;
	std::unordered_map<int, std::string> idToNode;
	std::unordered_map<int, std::string> idToRel;

	int maxNodeID = 0;
	int maxRelID = 0;
};

#endif // INDEX_H

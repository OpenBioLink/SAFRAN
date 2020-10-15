#ifndef INDEX_H
#define INDEX_H

#include <unordered_map>
#include <iostream>

class Index {

public:
	void addNode(std::string& nodesstring) {
		if (nodeToId.find(nodesstring) == nodeToId.end()) {
			nodeToId[nodesstring] = maxNodeID;
			idToNode[maxNodeID] = nodesstring;
			maxNodeID++;
		}
	}

	void addRelation(std::string& relstring) {
		if (relToId.find(relstring) == relToId.end()) {
			relToId[relstring] = maxRelID;
			idToRel[maxRelID] = relstring;
			maxRelID++;
		}
	}

	int getNodeSize() {
		return nodeToId.size();
	}

	int getRelSize() {
		return relToId.size();
	}

	int* getIdOfNodestring(std::string& node) {
		auto it = nodeToId.find(node);
		if (it != nodeToId.end()) {
			return &(it->second);
		}
		else {
			throw std::runtime_error(("Error: Node " + node + " not found in Trainingset").c_str());
		}
	}

	std::string* getStringOfNodeId(int& id) {
		auto it = idToNode.find(id);
		if (it != idToNode.end()) {
			return &(it->second);
		}
		else {
			std::cout << "Error: Id " << id << " not found in idToNode set";
			exit(-1);
		}
	}

	int* getIdOfRelationstring(std::string& relation) {
		auto it = relToId.find(relation);
		if (it != relToId.end()) {
			return &(it->second);
		}
		else {
			std::cout << "Error: Relation " << relation << " not found in Trainingset";
			exit(-1);
		}
	}

	std::string* getStringOfRelId(int& id) {
		auto it = idToRel.find(id);
		if (it != idToRel.end()) {
			return &(it->second);
		}
		else {
			std::cout << "Error: Id " << id << " not found in idToRel set";
			exit(-1);
		}
	}

	void rehash() {
		nodeToId.rehash(nodeToId.size());
		relToId.rehash(relToId.size());
		idToNode.rehash(idToNode.size());
		idToRel.rehash(idToRel.size());
	}

private:
	std::unordered_map<std::string, int> nodeToId;
	std::unordered_map<std::string, int> relToId;
	std::unordered_map<int, std::string> idToNode;
	std::unordered_map<int, std::string> idToRel;

	int maxNodeID = 0;
	int maxRelID = 0;
};

#endif // INDEX_H

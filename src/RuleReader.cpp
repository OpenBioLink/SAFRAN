#include "RuleReader.h"

RuleReader::RuleReader(std::string filepath, Index* index, TraintripleReader* graph) {
	this->graph = graph;
	this->index = index;
	read(filepath);
}

CSR<int, Rule> * RuleReader::getCSR() {
	return csr;
}

void RuleReader::read(std::string filepath) {
	int currID = 0;
	RelToRules rules;
	std::string line;
	std::ifstream myfile(filepath);
	if (myfile.is_open()) {
		while (!util::safeGetline(myfile, line).eof())
		{
			std::vector<std::string> rawrule = util::split(line, '\t');
			Rule * r = parseRule(rawrule, currID);
			currID++;
			if (r != nullptr) {
				//TODO no insert if rule bad, is probably never the cas (Rules are sampled from trainset)
				//r->toString();
				int* relationId = r->getHeadrelation();
				rules[*relationId].push_back(r);
			}
		}
		myfile.close();
	}
	else {
		std::cout << "Unable to open rule file " << filepath << std::endl;
		exit(-1);
	}

	csr = new CSR<int, Rule>(index->getRelSize(), rules);
}

Rule* RuleReader::parseRule(std::vector<std::string> rule, int currID) {
	Rule * ruleObj = new Rule(std::stoi(rule[0]), std::stoi(rule[1]), std::stod(rule[2]));

	std::string rawrule = rule[3];
	ruleObj->setRulestring(rawrule);
	ruleObj->setID(currID);
	std::stringstream ss(rawrule);

	Ruletype type = Ruletype::None;

	//Parse head
	std::string token;
	std::getline(ss, token, '(');
	int * relid = nullptr;
	try {
		relid = index->getIdOfRelationstring(token);
	}
	catch (std::runtime_error& e) {
		return nullptr;
	}
	if (relid == nullptr) {
		throw "Id not found in relid's";
	}
	ruleObj->setHeadrelation(relid);

	std::string head_tail;
	std::getline(ss, head_tail, ' ');

	std::string head, tail;
	std::tie(head, tail) = getHeadTail(head_tail);

	if (head != std::string("X")) {
		int* nodeid = nullptr;
		try {
			nodeid = index->getIdOfNodestring(head);
		}
		catch (std::runtime_error& e) {
			return nullptr;
		}
		if (nodeid == nullptr) {
			throw "Id not found in nodeid's";
		}
		ruleObj->setHeadconstant(nodeid);
	}
	else {
		type = Ruletype::XRule;
		ruleObj->setRuletype(type);
	}
	if (tail != std::string("Y")) {
		int* nodeid = nullptr;
		try {
			nodeid = index->getIdOfNodestring(tail);
		}
		catch (std::runtime_error& e) {
			return nullptr;
		}
		if (nodeid == nullptr) {
			throw "Id not found in nodeid's";
		}
		ruleObj->setHeadconstant(nodeid);
	}
	else {
		if (type == Ruletype::XRule) {
			type = Ruletype::XYRule;
			ruleObj->setRuletype(type);
		}
		else {
			type = Ruletype::YRule;
			ruleObj->setRuletype(type);
		}
	}

	//skip '<=' sign
	//std::getline(ss, token, ' ');
	std::getline(ss, token, ' ');
	//read whole body as string and split
	std::getline(ss, token); 
	std::vector<std::string> atoms = util::split(token, ' ');

	int rulelength = atoms.size();
	ruleObj->setRulelength(rulelength);
	int * forwardrelations = new int[rulelength];
	int* backwardrelations = new int[rulelength];

	// Get if xToY based on first atom
	bool xToY = parseXtoY(type, head, tail);

	// next is variable to start direction and relation parsing
	std::string next;
	if (type == Ruletype::XYRule || type == Ruletype::XRule) {
		next = "X";
	}
	else {
		next = "Y";
	}

	// If XRule or XYRule && atoms have xToY (left to right) direction
	// or it is a YRule && atoms have yToX direction
	for (int i = 0; i < rulelength; i++) {
		int index = i;
		//if (!((xToY && type != Ruletype::YRule) || (!xToY && type == Ruletype::YRule))) {
		if (xToY && type == Ruletype::YRule || (!xToY && (type == Ruletype::XRule || type == Ruletype::XYRule))) {
			index = rulelength - i - 1;
		}
		std::string atom = atoms[index];
		if (atom[atom.size() - 1] == ',') {
			atom.pop_back();
		}
		try {
			next = getRelation(atom, next, &forwardrelations[i]);
		}
		catch (std::runtime_error& e) {
			return nullptr;
		}
		if (!(next.length() == 1 && isupper(next[0])) && i == rulelength - 1 && (type == Ruletype::XRule || type == Ruletype::YRule)) {
			int* id = nullptr;
			try {
				id = this->index->getIdOfNodestring(next);
			}
			catch (std::runtime_error& e) {
				return nullptr;
			}
			if (id != nullptr) {
				ruleObj->setBodyconstantId(id);
			}
			else {
				throw "Something happend";
			}
		}
	}

	// Inverse relations and atoms
	for (int i = 0; i < rulelength; i++) {
		int indexFwd = rulelength - i - 1;
		if (forwardrelations[indexFwd] % 2 == 0) {
			backwardrelations[i] = forwardrelations[indexFwd] + 1;
		}
		else {
			backwardrelations[i] = forwardrelations[indexFwd] - 1;
		}
	}
	ruleObj->setRelationsFwd(forwardrelations);
	ruleObj->setRelationsBwd(backwardrelations);
	ruleObj->compute_body_hash();

	return ruleObj;
}

bool RuleReader::parseXtoY(Ruletype type, std::string& head, std::string& tail) {
	if (type == Ruletype::XYRule || type == Ruletype::XRule) {
		if (head == "X" || tail == "X") {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		//Ruletype YRule
		if (head == "Y" || tail == "Y") {
			return false;
		}
		else {
			return true;
		}
	}
}

std::string RuleReader::getRelation(std::string atom, std::string previous, int * relation) {
	std::stringstream ss(atom);
	std::string token;
	std::getline(ss, token, '(');
	int* relationId = nullptr;
	try {
		relationId = this->index->getIdOfRelationstring(token);
	}
	catch (std::runtime_error& e) {
		throw std::runtime_error("No relation found");
	}
	if (relationId == nullptr) {
		throw std::runtime_error("No relation found");
	}

	std::string head_tail;
	std::getline(ss, head_tail);
	
	std::string head, tail;
	std::tie(head, tail) = getHeadTail(head_tail);

	if (head == previous) {
		*relation = *relationId * 2;
		return tail;
	}
	else if (tail == previous) {
		*relation = *relationId * 2 + 1;
		return head;
	}
	else {
		throw "Error parsing direction";
	}
}

// input "asdf,bsdf)"
std::pair<std::string, std::string> RuleReader::getHeadTail(std::string& atom) {
	
	std::regex e("[A-D,X,Y],.*\\)");

	std::string head, tail;
	if (std::regex_match(atom, e)) {
		head = atom.substr(0, 1);
		tail = atom.substr(2, atom.size() - 3);
	}
	else {
		head = atom.substr(0, atom.size() - 3);
		tail = atom.substr(atom.size() - 2, 1);
	}
	return std::make_pair(head, tail);
}

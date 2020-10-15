#ifndef RULEREADER_H
#define RULEREADER_H

#include <string>
#include <iostream>
#include <fstream>

#include "TraintripleReader.hpp"
#include "CSR.hpp"
#include "Types.h"
#include "Util.hpp"
#include "Rule.hpp"
#include <omp.h>


class RuleReader
{
    public:
		RuleReader(std::string filepath, Index* index, TraintripleReader* graph) {
			this->graph = graph;
			this->index = index;
			read(filepath);
		}

		CSR<int, Rule> * getCSR() {
			return csr;
		}

    protected:

    private:
		TraintripleReader* graph;
		Index * index;
		CSR<int, Rule> * csr;

		void read(std::string filepath) {
			RelToRules rules;
			std::string line;
			std::ifstream myfile(filepath);
			if (myfile.is_open()) {
				while (getline(myfile, line))
				{
					std::vector<std::string> rawrule = util::split(line, '\t');
					Rule * r = parseRule(rawrule);
					//TODO no insert if rule bad, is probably never the cas (Rules are sampled from trainset)
					//r->toString();
					int * relationId = r->getHeadrelation();
					rules[*relationId].insert(r);
				}
				myfile.close();
			}
			else {
				std::cout << "Unable to open rule file " << filepath << std::endl;
				exit(-1);
			}


			csr = new CSR<int, Rule>(index->getRelSize(), rules);
		}

		Rule* parseRule(std::vector<std::string> rule) {
			Rule * ruleObj = new Rule(std::stoi(rule[0]), std::stoi(rule[1]), std::stod(rule[2]));

			std::string rawrule = rule[3];
			ruleObj->setRulestring(rawrule);
			std::stringstream ss(rawrule);

			Ruletype type = Ruletype::None;

			//Parse head
			std::string token;
			std::getline(ss, token, '(');
			int * relid = index->getIdOfRelationstring(token);
			if (relid == nullptr) {
				throw "Id not found in relid's";
			}
			ruleObj->setHeadrelation(relid);
			std::getline(ss, token, ',');
			if (token != std::string("X")) {
				int * nodeid = index->getIdOfNodestring(token);
				if (nodeid == nullptr) {
					throw "Id not found in nodeid's";
				}
				ruleObj->setHeadconstant(nodeid);
			}
			else {
				type = Ruletype::XRule;
				ruleObj->setRuletype(type);
			}
			std::getline(ss, token, ')');
			if (token != std::string("Y")) {
				int * nodeid = index->getIdOfNodestring(token);
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
			std::getline(ss, token, ' ');
			std::getline(ss, token, ' ');
			//read whole body as string and split
			std::getline(ss, token);
			std::vector<std::string> atoms = util::split(token, ' ');

			int rulelength = atoms.size();
			ruleObj->setRulelength(rulelength);
			int * forwardrelations = new int[rulelength];
			int* backwardrelations = new int[rulelength];

			// Get if xToY based on first atom
			bool xToY = parseXtoY(type, atoms[0]);

			// next is variable to start direction and relation parsing
			std::string next;
			if (type == Ruletype::XYRule || type == Ruletype::XRule) {
				next = "X";
			}
			else {
				next = "Y";
			}

			// If XRule or XYRule and atoms have xToY (left to right) direction
			// or it is a YRule and atoms have yToX direction
			for (int i = 0; i < rulelength; i++) {
				int index = i;
				//if (!((xToY && type != Ruletype::YRule) || (!xToY && type == Ruletype::YRule))) {
				if (xToY && type == Ruletype::YRule || (!xToY && (type == Ruletype::XRule || type == Ruletype::XYRule))) {
					index = rulelength - i - 1;
				}
				next = getRelation(atoms[index], next, &forwardrelations[i]);
				if (!(next.length() == 1 && isupper(next[0])) && i == rulelength - 1 && (type == Ruletype::XRule || type == Ruletype::YRule)) {
					int * id = this->index->getIdOfNodestring(next);
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

		bool parseXtoY(Ruletype type, std::string atom) {
			std::string token;
			std::stringstream atomss(atom);
			std::getline(atomss, token, '(');
			std::getline(atomss, token, ')');
			std::vector<std::string> entities = util::split(token, ',');
			if (type == Ruletype::XYRule || type == Ruletype::XRule) {
				if (entities[0] == "X" || entities[1] == "X") {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				//Ruletype YRule
				if (entities[0] == "Y" || entities[1] == "Y") {
					return false;
				}
				else {
					return true;
				}
			}
		}

		std::string getRelation(std::string atom, std::string previous, int * relation) {
			std::stringstream ss(atom);
			std::string token;
			std::getline(ss, token, '(');
			int * relationId = this->index->getIdOfRelationstring(token);
			if (relationId == nullptr) {
				throw "No relation found";
			}
			std::getline(ss, token, ')');
			std::vector<std::string> a = util::split(token, ',');
			if (a[0] == previous) {
				*relation = *relationId * 2;
				return a[1];
			}
			else if (a[1] == previous) {
				*relation = *relationId * 2 + 1;
				return a[0];
			}
			else {
				throw "Error parsing direction";
			}
		}
};

#endif // RULEREADER_H

#ifndef RULEREADER_H
#define RULEREADER_H

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <stdio.h>

#include "Graph.hpp"
#include "Rule.hpp"
#include "Util.hpp"

#define cudaCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                msg, cudaGetErrorString(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

class RuleReader
{
    public:
		__host__ RuleReader(std::string filepath, Graph * graph, bool cuda) {
			this->graph = graph;
			read(filepath, cuda);
		}

		__host__ int* getRuleAdjBegin() {
			return adj_begin;
		}

		__host__ Rule* getRulesAdjList() {
			return adj_list;
		}

    protected:

    private:
		Graph * graph;
        std::unordered_map<int, std::unordered_set<Rule*>> rules;
		int* adj_begin;
		Rule* adj_list;

		__host__ Rule* parseRule(std::vector<std::string> rule) {
			Rule * ruleObj = new Rule(std::stoi(rule[0]), std::stoi(rule[1]), std::stod(rule[2]));

			std::string rawrule = rule[3];
			std::stringstream ss(rawrule);

			Ruletype type = Ruletype::None;

			//Parse head
			std::string token;
			std::getline(ss, token, '(');
			int * relid = graph->getIdOfRelationstring(token);
			if (relid == nullptr) {
				throw "Id not found in relid's";
			}
			ruleObj->setHeadrelation(relid);
			std::getline(ss, token, ',');
			if (token != std::string("X")) {
				int * nodeid = graph->getIdOfNodestring(token);
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
				int * nodeid = graph->getIdOfNodestring(token);
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
					int * id = this->graph->getIdOfNodestring(next);
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

			return ruleObj;
		}

		__host__ bool parseXtoY(Ruletype type, std::string atom) {
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

		__host__ std::string getRelation(std::string atom, std::string previous, int * relation) {
			std::stringstream ss(atom);
			std::string token;
			std::getline(ss, token, '(');
			int * relationId = this->graph->getIdOfRelationstring(token);
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

		__host__ void read(std::string filepath, bool cuda){
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
			else std::cout << "Unable to open file";
			generateCSR(cuda);
		}

		__host__ void generateCSR(bool cuda) {
			int numRel = graph->getRelToId()->size();
			int numRules = rules.size();

			int nnz = 0;
			for (int i = 0; i < numRel; i++) {
				if (rules.find(i) != rules.end()) {
					nnz += rules.find(i)->second.size();
				}
			}

			if (cuda) {
				cudaMallocManaged(&adj_begin, (3 + numRel + 1) * sizeof(int));
				cudaCheckErrors("adj_begin");
				cudaMallocManaged(&adj_list, nnz * sizeof(Rule));
				cudaCheckErrors("adj_list");
			}
			else {
				adj_begin = new int[3 + numRel + 1];
				adj_list = new Rule[nnz];
			}
			int indptrlen = numRel + 1;
			int indlen = nnz;
			int len = 3 + indptrlen + indlen;

			adj_begin[0] = len;
			adj_begin[1] = indptrlen;
			adj_begin[2] = indlen;
			int* indptr = &adj_begin[3];
			Rule* ind = &adj_list[0];
			*indptr = 0;
			for (int rel = 0; rel < numRel; rel++) {
				if (rules.find(rel) != rules.end()) {
					*(indptr + 1) = *indptr + rules[rel].size();
					indptr++;
					int i = 0;
					auto it = rules[rel].begin();
					while (it != rules[rel].end())
					{
						ind[i] = **it;
						i++;
						it++;
					}
					std::sort(ind, ind + rules[rel].size(), ruleComp);
					ind = ind + rules[rel].size();
				}
				else {
					*(indptr + 1) = *indptr;
					indptr++;
				}
			}
		}

		struct {
			bool operator()(Rule a, Rule b) const
			{
				return a.getAppliedConfidence() < b.getAppliedConfidence();
			}
		} ruleComp;
};

#endif // RULEREADER_H

#ifndef CSR_H
#define CSR_H

#include "Types.h"
#include <algorithm>

template<class A, class B> class CSR {
public:

	CSR(int relationSize, int nodeSize, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads) {
		generateTripleCSR(relationSize, nodeSize, relHeadToTails, relTailToHeads);
	}

	CSR(int relationSize, RelToRules& rules) {
		generateRuleCSR(relationSize, rules);
	}

	A* getAdjBegin() {
		return adj_begin;
	}
	B* getAdjList() {
		return adj_list;
	}
private:
	A* adj_begin;
	B* adj_list;

	void generateTripleCSR(int relationSize, int nodeSize, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads) {
		int nRelsNNZ = 0;
		int nnz = 0;
		int* indlen = new int[relationSize * 2];
		for (int i = 0; i < relationSize; i++) {
			// If rel present in headToTails it should be also present in tailToHeads
			if (relHeadToTails.find(i) != relHeadToTails.end()) {
				nRelsNNZ++;
			}
			int indlenHTT = 0;
			int indlenTTH = 0;
			for (int j = 0; j < nodeSize; j++) {
				if (relHeadToTails.find(i) != relHeadToTails.end()) {
					if (relHeadToTails[i].find(j) != relHeadToTails[i].end()) {
						indlenHTT = indlenHTT + relHeadToTails[i][j].size();
					}
				}
				if (relTailToHeads.find(i) != relTailToHeads.end()) {
					if (relTailToHeads[i].find(j) != relTailToHeads[i].end()) {
						indlenTTH = indlenTTH + relTailToHeads[i][j].size();
					}
				}
			}
			indlen[i * 2] = indlenHTT;
			indlen[i * 2 + 1] = indlenTTH;
			nnz += indlenHTT;
			nnz += indlenTTH;
		}
		int adj_list_size = nRelsNNZ * 2 * (3 + nodeSize + 1) + nnz;
		adj_list = new int[adj_list_size];
		int adj_begin_size = relationSize * 2;
		adj_begin = new int[adj_begin_size];

		int start = 0;
		for (int rel = 0; rel < relationSize; rel++) {
			//if rel not in relHeadToTails it is also not in relTailToHeads
			if (relHeadToTails.find(rel) != relHeadToTails.end()) {
				adj_begin[rel * 2] = start;
				int currentIndptrlen = nodeSize + 1;
				int currentIndlen = indlen[rel * 2];
				int currentLen = 3 + currentIndptrlen + currentIndlen;

				int stop = start + currentLen;
				adj_list[start] = currentLen;
				adj_list[start + 1] = currentIndptrlen;
				adj_list[start + 2] = currentIndlen;

				int* indptr = &adj_list[start + 3];
				int* ind = &adj_list[start + 3 + currentIndptrlen];

				*indptr = 0;
				for (int node = 0; node < nodeSize; node++) {
					if (relHeadToTails[rel].find(node) != relHeadToTails[rel].end()) {
						*(indptr + 1) = *indptr + relHeadToTails[rel][node].size();
						indptr++;
						auto it = relHeadToTails[rel][node].begin();
						while (it != relHeadToTails[rel][node].end())
						{
							*ind = *it;
							ind++;
							it++;
						}
					}
					else {
						*(indptr + 1) = *indptr;
						indptr++;
					}
				}
				start = stop;


				adj_begin[rel * 2 + 1] = start;
				currentIndptrlen = nodeSize + 1;
				currentIndlen = indlen[rel * 2 + 1];
				currentLen = 3 + currentIndptrlen + currentIndlen;

				stop = start + currentLen;

				adj_list[start] = currentLen;
				adj_list[start + 1] = currentIndptrlen;
				adj_list[start + 2] = currentIndlen;

				indptr = &adj_list[start + 3];
				ind = &adj_list[start + 3 + currentIndptrlen];

				*indptr = 0;
				for (int node = 0; node < nodeSize; node++) {
					if (relTailToHeads[rel].find(node) != relTailToHeads[rel].end()) {
						*(indptr + 1) = *indptr + relTailToHeads[rel][node].size();
						indptr++;
						auto it = relTailToHeads[rel][node].begin();
						while (it != relTailToHeads[rel][node].end())
						{
							*ind = *it;
							ind++;
							it++;
						}
					}
					else {
						*(indptr + 1) = *indptr;
						indptr++;
					}
				}
				start = stop;
			}
			else {
				adj_begin[rel * 2] = -1;
				adj_begin[rel * 2 + 1] = -1;
			}
		}
	}

	void generateRuleCSR(int& relationSize, RelToRules& rules) {

		int numRules = rules.size();

		int nnz = 0;
		for (int i = 0; i < relationSize; i++) {
			if (rules.find(i) != rules.end()) {
				nnz += rules.find(i)->second.size();
			}
		}


		adj_begin = new int[3 + relationSize + 1];
		adj_list = new Rule[nnz];

		int indptrlen = relationSize + 1;
		int indlen = nnz;
		int len = 3 + indptrlen + indlen;

		adj_begin[0] = len;
		adj_begin[1] = indptrlen;
		adj_begin[2] = indlen;
		int* indptr = &adj_begin[3];
		Rule* ind = &adj_list[0];
		*indptr = 0;
		for (int rel = 0; rel < relationSize; rel++) {
			if (rules.find(rel) != rules.end()) {
				*(indptr + 1) = *indptr + rules[rel].size();
				indptr++;
				std::copy(rules[rel].begin(), rules[rel].end(), ind);
				std::sort(ind, ind + rules[rel].size(), ruleComp);
				ind = ind + rules[rel].size();
			}
			else {
				*(indptr + 1) = *indptr;
				indptr++;
			}
			for (auto p : rules[rel])
			{
				delete p;
			}
		}
	}

	struct {
		bool operator()(Rule a, Rule b) const
		{
			if (*a.getHeadrelation() != *b.getHeadrelation()) {
				throw std::runtime_error("HI");
			}
			return a.getAppliedConfidence() > b.getAppliedConfidence();
		}
	} ruleComp;
};


#endif // !CSR_H

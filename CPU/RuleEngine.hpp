#ifndef RULEENGINE_H
#define RULEENGINE_H

#ifdef __unix
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

#include "TraintripleReader.hpp"
#include "RuleReader.hpp"
#include "TesttripleReader.hpp"
#include "ScoreTree.hpp"
#include "ValidationtripleReader.hpp"
#include "Properties.hpp"

#include <iomanip>
#include <thread>
#include <mutex>

class RuleEngine {

public:

	RuleEngine(const RuleEngine& that) {
		this->index = that.index;
		this->graph = that.graph;
		this->ttr = that.ttr;
		this->vtr = that.vtr;
		this->rr = that.rr;
		this->WORKER_THREADS = that.WORKER_THREADS;
		this->DISCRIMINATION_BOUND = that.DISCRIMINATION_BOUND;
		this->TRIAL_SIZE = that.TRIAL_SIZE;
		this->lock = that.lock;
		this->writeLock = that.writeLock;
	}

	RuleEngine(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
		this->index = index;
		this->graph = graph;
		this->ttr = ttr;
		this->vtr = vtr;
		this->rr = rr;
		this->lock = new std::mutex();
		this->writeLock = new std::mutex();

		WORKER_THREADS = Properties::get().WORKER_THREADS;
		TRIAL_SIZE = Properties::get().TRIAL_SIZE;
		DISCRIMINATION_BOUND = Properties::get().DISCRIMINATION_BOUND;
	}

	virtual void start() = 0;
	virtual void run(int threadId) = 0;




protected:
	Index* index;
	TraintripleReader* graph;
	TesttripleReader* ttr;
	ValidationtripleReader* vtr;
	RuleReader* rr;
	std::stringstream* out;

	FILE* pFile;

	int TRIAL_SIZE;
	int DISCRIMINATION_BOUND;
	int WORKER_THREADS;

	std::mutex* lock;

	void applyCyclicRule(int* adj_list_starts, int* adj_lists, int* val, int* relations, int rulelength, int N, std::vector<int>& results, int& counter, int* previous) {
		counter++;
		if (counter == TRIAL_SIZE) {
			return;
		}

		int value = *val;
		previous[N] = value;
		int* adj_list = &(adj_lists[adj_list_starts[*relations]]);
		int start_indptr = 3;
		int size_indptr = adj_list[1];
		int start_ind = start_indptr + size_indptr;
		//int size_ind = adj_list[2];


		relations++;
		N++;
		int ind_ptr = adj_list[start_indptr + value];
		int len = adj_list[start_indptr + value + 1] - ind_ptr;

		if (Properties::get().INTERMEDIATE_DISCRIMINATION == 1 and DISCRIMINATION_BOUND > 0) {
			if (len > DISCRIMINATION_BOUND) { return; }
		}

		if (N == rulelength) {
			for (int j = 0; j < len; j++)
			{
				int nextval = adj_list[start_ind + ind_ptr + j];

				bool inPrevious = false;
				for (int i = 0; i < N; i++) {
					if (previous[i] == nextval) {
						inPrevious = true;
					}
				}
				if (inPrevious) { continue; }

				results.push_back(nextval);
				if (results.size() == DISCRIMINATION_BOUND and DISCRIMINATION_BOUND > 0) {
					if (Properties::get().DISCRIMINATION_UNIQUE == 1) {
						std::sort(results.begin(), results.end());
						auto end = std::unique(results.begin(), results.end());
						results.erase(end, results.end());
					}
					// second if needed if after DISCRIMINATION_UNIQUE still *resultlength == DISCRIMINATION_BOUND
					if (results.size() == DISCRIMINATION_BOUND) {
						results.clear();
						return;
					}
				}
				counter++;
				if (counter == TRIAL_SIZE) {
					return;
				}
			}
		}
		else {
			for (int j = 0; j < len; j++)
			{
				int nextval = adj_list[start_ind + ind_ptr + j];
				bool inPrevious = false;
				for (int i = 0; i < N; i++) {
					if (previous[i] == nextval) {
						inPrevious = true;
					}
				}
				if (inPrevious) { continue; }
				applyCyclicRule(adj_list_starts, adj_lists, &nextval, relations, rulelength, N, results, counter, previous);
				//if (*resultlength == N_SAMPLES) return;
			}
		}
	}

	void applyAcyclicRule(int* adj_list_starts, int* adj_lists, int* relations, int rulelength, int N, std::vector<int>& results, int& counter, int* previous) {
		int* adj_list = &(adj_lists[adj_list_starts[*relations]]);
		int start_indptr = 3;
		int size_indptr = adj_list[1];
		int start_ind = start_indptr + size_indptr;
		//int size_ind = adj_list[2];

		relations++;
		N++;

		if (N == rulelength) {
			for (int val = 0; val < size_indptr - 1; val++) {
				previous[N - 1] = val;
				int ind_ptr = adj_list[start_indptr + val];
				int len = adj_list[start_indptr + val + 1] - ind_ptr;

				if (Properties::get().INTERMEDIATE_DISCRIMINATION == 1 and DISCRIMINATION_BOUND > 0) {
					if (len > DISCRIMINATION_BOUND) { continue; }
				}

				for (int j = 0; j < len; j++) {
					int nextval = adj_list[start_ind + ind_ptr + j];

					bool inPrevious = false;
					for (int i = 0; i < N; i++) {
						if (previous[i] == nextval) {
							inPrevious = true;
						}
					}
					if (inPrevious) { continue; }

					results.push_back(nextval);
					if (results.size() == DISCRIMINATION_BOUND and DISCRIMINATION_BOUND > 0) {
						if (Properties::get().DISCRIMINATION_UNIQUE == 1) {
							std::sort(results.begin(), results.end());
							auto end = std::unique(results.begin(), results.end());
							results.erase(end, results.end());
						}
						// second if needed if after DISCRIMINATION_UNIQUE still *resultlength == DISCRIMINATION_BOUND
						if (results.size() == DISCRIMINATION_BOUND) {
							results.clear();
							return;
						}
					}
					counter++;
					if (counter == TRIAL_SIZE) {
						return;
					}
				}
			}
		}
		else {
			for (int val = 0; val < size_indptr - 1; val++) {
				previous[N - 1] = val;
				int ind_ptr = adj_list[start_indptr + val];
				int len = adj_list[start_indptr + val + 1] - ind_ptr;

				if (Properties::get().INTERMEDIATE_DISCRIMINATION == 1 and DISCRIMINATION_BOUND > 0) {
					if (len > DISCRIMINATION_BOUND) { continue; }
				}

				for (int j = 0; j < len; j++) {
					int nextval = adj_list[start_ind + ind_ptr + j];

					bool inPrevious = false;
					for (int i = 0; i < N; i++) {
						if (previous[i] == nextval) {
							inPrevious = true;
						}
					}
					if (inPrevious) { continue; }

					applyCyclicRule(adj_list_starts, adj_lists, &nextval, relations, rulelength, N, results, counter, previous);
					//if (*resultlength == N_SAMPLES) return;
				}
			}
		}
	}

	bool existsAcyclic(int* adj_list_starts, int* adj_lists, int* valId, int* constant, int* relations, int N) {
		int* adj_list = &(adj_lists[adj_list_starts[*relations]]);
		int start_indptr = 3;
		int size_indptr = adj_list[1];
		int start_ind = start_indptr + size_indptr;
		//int size_ind = adj_list[2];

		relations++;
		N--;
		int ind_ptr = adj_list[start_indptr + *valId];
		int len = adj_list[start_indptr + *valId + 1] - ind_ptr;
		if (N == 0 && constant != nullptr) {
			for (int j = 0; j < len; j++) {
				int to = adj_list[start_ind + ind_ptr + j];
				if (to == *constant) {
					return true;
				}
			}
			return false;
		}
		else if (N == 0 && constant == nullptr) {
			if (len > 0) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			for (int j = 0; j < len; j++) {
				if (existsAcyclic(adj_list_starts, adj_lists, &adj_list[start_ind + ind_ptr + j], constant, relations, N)) {
					return true;
				};
			}
			return false;
		}
	}

	void computeHeadsCyclic(int* adj_list_starts, int* adj_lists, int* valId, Rule& rule, std::vector<int>& results) {
		int* previous = new int[rule.getRulelength()];
		int* relations = rule.getRelationsBwd();
		int counter = 0;
		applyCyclicRule(adj_list_starts, adj_lists, valId, relations, rule.getRulelength(), 0, results, counter, previous);
		delete[] previous;
	}
	void computeTailsCyclic(int* adj_list_starts, int* adj_lists, int* valId, Rule& rule, std::vector<int>& results) {
		int* previous = new int[rule.getRulelength()];
		int* relations = rule.getRelationsFwd();
		int counter = 0;
		applyCyclicRule(adj_list_starts, adj_lists, valId, relations, rule.getRulelength(), 0, results, counter, previous);
		delete[] previous;
	}
	void computeAcyclic(int* adj_list_starts, int* adj_lists, Rule& rule, std::vector<int>& results) {
		int* previous = new int[rule.getRulelength()];
		int* relations = rule.getRelationsBwd();
		int counter = 0;
		if (rule.getBodyconstantId() != nullptr) {
			applyCyclicRule(adj_list_starts, adj_lists, rule.getBodyconstantId(), relations, rule.getRulelength(), 0, results, counter, previous);
		}
		else {
			applyAcyclicRule(adj_list_starts, adj_lists, relations, rule.getRulelength(), 0, results, counter, previous);
		}
		delete[] previous;
	}
	bool existsAcyclic(int* adj_list_starts, int* adj_lists, int* valId, Rule& rule) {
		int* relations = rule.getRelationsFwd();
		int* constantnode = nullptr;
		if (rule.getBodyconstantId() != nullptr) {
			constantnode = rule.getBodyconstantId();
		}
		return existsAcyclic(adj_list_starts, adj_lists, valId, constantnode, relations, rule.getRulelength());
	}

	/*
	void writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, double>> headresults, std::vector<std::pair<int, double>> tailresults, std::stringstream& out, int& K) {
		out << *(index->getStringOfNodeId(head)) << " " << *(index->getStringOfRelId(rel)) << " " << *(index->getStringOfNodeId(tail)) << std::endl;
		out << "Heads: ";
		int maxHead = headresults.size() < K ? headresults.size() : K;
		for (int i = 0; i < maxHead; i++) {
			out << *(index->getStringOfNodeId(headresults[i].first)) << "\t" << std::setprecision(16) << headresults[i].second << "\t";
		}
		out << std::endl;
		out << "Tails: ";
		int maxTail = tailresults.size() < K ? tailresults.size() : K;
		for (int i = 0; i < maxTail; i++) {
			out << *(index->getStringOfNodeId(tailresults[i].first)) << "\t" << std::setprecision(16) << tailresults[i].second << "\t";
		}
		out << std::endl;
	}
	*/

	std::mutex* writeLock;
	void writeTopKCandidates(int head, int rel, int tail, std::vector<std::pair<int, double>> headresults, std::vector<std::pair<int, double>> tailresults, FILE* pFile, int& K) {
		writeLock->lock();
		fprintf(pFile, "%s %s %s\nHeads: ", index->getStringOfNodeId(head)->c_str(), index->getStringOfRelId(rel)->c_str(), index->getStringOfNodeId(tail)->c_str());

		int maxHead = headresults.size() < K ? headresults.size() : K;
		for (int i = 0; i < maxHead; i++) {
			fprintf(pFile, "%s\t%.16f\t", index->getStringOfNodeId(headresults[i].first)->c_str(), headresults[i].second);
		}
		fprintf(pFile, "\nTails: ");
		int maxTail = tailresults.size() < K ? tailresults.size() : K;
		for (int i = 0; i < maxTail; i++) {
			fprintf(pFile, "%s\t%.16f\t", index->getStringOfNodeId(tailresults[i].first)->c_str(), tailresults[i].second);
		}
		fprintf(pFile, "\n");
		writeLock->unlock();
	}

	struct {
		bool operator()(std::pair<int, double> const& a, std::pair<int, double> const& b) const
		{
			return a.second > b.second;
		}
	} finalResultComperator;
};

#endif // RULEENGINE_H
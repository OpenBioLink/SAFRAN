#ifndef SCORETREE_H
#define SCORETREE_H

#define LOWER_BOUND 10
#define UPPER_BOUND 10
#define EPSILON 0.0001

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <cmath>

class ScoreTree
{
public:

	ScoreTree() {
		this->score = 0.0;
		this->storedValues = nullptr;
		this->nStoredValues = 0;
		this->closed = false;
		this->numOfValues = 0;
		this->index = 0;
		this->root = true;
	};

	void Free() {
		for (int i = 0; i < children.size(); i++) {
			children[i].Free();
		}
		delete[] storedValues;
	}

	ScoreTree(double score, int * values, int valuelength, int index) {
		this->score = score;
		nStoredValues = valuelength;
		storedValues = new int[nStoredValues];
		for (int i = 0; i < nStoredValues; i++) {
			storedValues[i] = values[i];
		}
		if (nStoredValues <= 1) this->closed = true;
		else this->closed = false;
		this->numOfValues = valuelength;
		this->root = false;
		this->index = index;
	}

	bool fine() {
		if (root && children.size() > 0) {
			int i = children[children.size() - 1].index;
			if (i >= LOWER_BOUND && i <= UPPER_BOUND) {
				return isFirstUnique();
			}
		}
		return false;
	}

	bool isFirstUnique() {
		ScoreTree * tree = this;
		while (tree->children.size() > 0) {
			tree = &(tree->children[0]);
		}
		return tree->closed;
	}

	void addValues(double score, int * values, int nValues) {
		addValues(score, values, nValues, 0);
	}

	void print(int intent) {
		for (int i = 0; i < intent * 4; i++) {
			printf(" ");
		}
		intent++;
		printf("%d %f [%d](%d) -> { ", closed, score, index, numOfValues);
		if (storedValues != nullptr) {
			for (int i = 0; i < nStoredValues; i++) {
				printf("%d ", storedValues[i]);
			}
		}
		printf("}\n");

		for (int i = 0; i < children.size(); i++) {
			children[i].print(intent);
		}
	}

	void getResults(std::vector<std::pair<int, double>>& results, double ps, int level) {
		if (children.size() > 0) {
			for (int i = 0; i < children.size(); i++) {
				if (root) {
					children[i].getResults(results, ps, level + 1);
				}
				else {
					double psUpdated = ps + std::powf(EPSILON, level - 1) * score;
					children[i].getResults(results, psUpdated, level + 1);
				}
			}
		}
		if (!root) {
			double psUpdated = ps + std::powf(EPSILON, level - 1) * score;
			for(int i = 0; i < nStoredValues; i++){
				results.push_back(std::pair<int, double>(storedValues[i], psUpdated));
			}
		}
	}

	void getResults(std::vector<std::pair<int, double>>& results) {
		getResults(results, 0, 0);
	}

protected:

private:
	double score;
	int numOfValues;
	int index;
	bool root;
	bool closed;
	int * storedValues;
	int nStoredValues;
	std::vector<ScoreTree> children;

	void addValues(double score, int * values, int& nValues, int counter) {
		for (int i = 0; i < children.size(); i++) {
			children[i].addValues(score, values, nValues, 0);
		}

		int * touched_end;
		int * untouched_end;
		int * values_end;
		int * touched = new int[1000];
		int * untouched = new int[1000];
		int nTouched = 0;
		int nUntouched = 0;

		if (!root) {
			touched_end = std::set_intersection(storedValues, storedValues + nStoredValues, values, values + nValues, touched);
			nTouched = std::distance(touched, touched_end);
			untouched_end = std::set_difference(storedValues, storedValues + nStoredValues, values, values + nValues, untouched);
			nUntouched = std::distance(untouched, untouched_end);
			values_end = std::set_difference(values, values + nValues, touched, touched_end, values);
			nValues = std::distance(values, values_end);
		}


		if (nTouched > 0 && nStoredValues > 1 && nTouched < nStoredValues) {
			int childIndex = index - nUntouched;
			if (childIndex >= LOWER_BOUND) {
				nStoredValues = nTouched;
				if (storedValues != nullptr) { delete[] storedValues; }
				storedValues = new int[nStoredValues];
				for (int i = 0; i < nStoredValues; i++) {
					storedValues[i] = touched[i];
				}
				index = childIndex;
				numOfValues -= nUntouched;
			}
			else {
				nStoredValues = nUntouched;
				if (storedValues != nullptr) { delete[] storedValues; }
				storedValues = new int[nStoredValues];
				for (int i = 0; i < nStoredValues; i++) {
					storedValues[i] = untouched[i];
				}
				addChild(score, touched, nTouched, childIndex);
			}
		}

		if (root == true && nValues > 0 && numOfValues < LOWER_BOUND) {
			addChild(score, values, nValues, numOfValues + nValues);
			numOfValues += nValues;
		}

		if (storedValues == nullptr || nStoredValues <= 1) {
			bool c = true;
			for (int i = 0; i < children.size(); i++) {
				if (children[i].closed == false) {
					c = false;
					break;
				}
			}
			closed = c;
		}

		delete[] touched;
		delete[] untouched;
	}

	ScoreTree addChild(double score, int * values, int valuelength, int childIndex) {
		ScoreTree child = ScoreTree(score, values, valuelength, childIndex);
		children.push_back(child);
		return child;
	}


};

#endif // SCORETREE_H
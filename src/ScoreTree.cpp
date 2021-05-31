#include "ScoreTree.h"

ScoreTree::ScoreTree() {
	this->score = 0.0;
	this->storedValues = nullptr;
	this->nStoredValues = 0;
	this->closed = false;
	this->numOfValues = 0;
	this->index = 0;
	this->root = true;
	this->LOWER_BOUND = this->UPPER_BOUND = Properties::get().TOP_K_OUTPUT;
};

void ScoreTree::Free() {
	for (int i = 0; i < children.size(); i++) {
		children[i].Free();
	}
	delete[] storedValues;
}

ScoreTree::ScoreTree(double score, int * values, int valuelength, int index) {
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
	this->LOWER_BOUND = this->UPPER_BOUND = Properties::get().TOP_K_OUTPUT;
}

bool ScoreTree::fine() {
	if (root && children.size() > 0) {
		int i = children[children.size() - 1].index;
		if (i >= LOWER_BOUND && i <= UPPER_BOUND) {
			return isFirstUnique();
		}
	}
	return false;
}

bool ScoreTree::isFirstUnique() {
	ScoreTree * tree = this;
	while (tree->children.size() > 0) {
		tree = &(tree->children[0]);
	}
	return tree->closed;
}

void ScoreTree::addValues(double score, int * values, int nValues) {
	addValues(score, values, nValues, 0);
}

void ScoreTree::print(int intent, Index * index_obj) {
	for (int i = 0; i < intent * 4; i++) {
		printf(" ");
	}
	intent++;
	printf("%d %f [%d](%d) -> { ", closed, score, index, numOfValues);
	if (storedValues != nullptr) {
		for (int i = 0; i < nStoredValues; i++) {
			printf("%s ", (*(index_obj->getStringOfNodeId(storedValues[i]))).c_str());
		}
	}
	printf("}\n");

	for (int i = 0; i < children.size(); i++) {
		children[i].print(intent, index_obj);
	}
}

void ScoreTree::getResults(std::vector<std::pair<int, double>>& results, double ps, int level) {
	if (children.size() > 0) {
		for (int i = 0; i < children.size(); i++) {
			if (root) {
				children[i].getResults(results, ps, level + 1);
			}
			else {
				double psUpdated = ps + std::pow(EPSILON, level - 1) * score;
				children[i].getResults(results, psUpdated, level + 1);
			}
		}
	}
	if (!root) {
		double psUpdated = ps + std::pow(EPSILON, level - 1) * score;
		for(int i = 0; i < nStoredValues; i++){
			results.push_back(std::pair<int, double>(storedValues[i], psUpdated));
		}
	}
}

void ScoreTree::getResults(std::vector<std::pair<int, double>>& results) {
	getResults(results, 0, 0);
}

void ScoreTree::addValues(double score, int * values, int& nValues, int counter) {
	//std::sort(values, values + nValues);
	for (int i = 0; i < children.size(); i++) {
		children[i].addValues(score, values, nValues, 0);
	}

	int * untouched_end;
	int * values_end;
	std::vector<int> touched;
	std::vector<int> untouched;

	if (!root) {
		std::set_intersection(storedValues, storedValues + nStoredValues, values, values + nValues, std::back_inserter(touched));
		std::set_difference(storedValues, storedValues + nStoredValues, values, values + nValues, std::back_inserter(untouched));
		values_end = std::set_difference(values, values + nValues, touched.begin(), touched.end(), values);
		nValues = std::distance(values, values_end);
	}


	if (touched.size() > 0 && ((nStoredValues > 1 && touched.size() < nStoredValues) || (score == this->score && touched.size() <= nStoredValues))){
		int childIndex = index - untouched.size();
		if (childIndex >= LOWER_BOUND) {
			nStoredValues = touched.size();
			if (storedValues != nullptr) { delete[] storedValues; }
			storedValues = new int[nStoredValues];
			for (int i = 0; i < nStoredValues; i++) {
				storedValues[i] = touched[i];
			}
			index = childIndex;
			numOfValues -= untouched.size();
		}
		else {
			nStoredValues = untouched.size();
			if (storedValues != nullptr) { delete[] storedValues; }
			storedValues = new int[nStoredValues];
			for (int i = 0; i < nStoredValues; i++) {
				storedValues[i] = untouched[i];
			}
			addChild(score, &touched[0], touched.size(), childIndex);
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
}

ScoreTree ScoreTree::addChild(double score, int * values, int valuelength, int childIndex) {
	ScoreTree child = ScoreTree(score, values, valuelength, childIndex);
	children.push_back(child);
	return child;
}

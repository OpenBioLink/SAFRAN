#ifndef SCORETREE_H
#define SCORETREE_H

#define EPSILON 0.0001

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <vector>

#include "Index.h"
#include "Properties.hpp"

#include <iostream>
#include<iomanip>
#include<limits>

class ScoreTree
{
public:

	ScoreTree();
	ScoreTree(double score, int* values, int valuelength, int index);

	void Free();
	bool fine();
	bool isFirstUnique();
	void addValues(double score, int* values, int nValues);
	void print(int intent, Index* index_obj);
	void getResults(std::vector<std::pair<int, double>>& results, double ps, int level);
	void getResults(std::vector<std::pair<int, double>>& results);

protected:

private:
	double score;
	int numOfValues;
	int index;
	bool root;
	bool closed;
	int* storedValues;
	int nStoredValues;
	std::vector<ScoreTree> children;

	int LOWER_BOUND;
	int UPPER_BOUND;

	void addValues(double score, int* values, int& nValues, int counter);
	ScoreTree addChild(double score, int* values, int valuelength, int childIndex);

};

#endif // SCORETREE_H

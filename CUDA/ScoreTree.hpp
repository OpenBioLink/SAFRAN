#ifndef SCORETREE_H
#define SCORETREE_H
#define LOWER_BOUND 10
#define UPPER_BOUND 10
#define EPSILON 0.0001
#define CHILDREN_CAPACITY 10

#include "cuda_runtime.h"

class ScoreTree
{
	public:
		
		__device__ ScoreTree() {};

		__device__ void Free() {
			for (int i = 0; i < nChildren; i++) {
				children[i].Free();
			}
			delete[] children;
		}

		__device__ void initializeScoreTree() {
			this->children = new ScoreTree[CHILDREN_CAPACITY];
			this->nChildren = 0;
			this->nStoredValues = 0;
			this->storedValues = nullptr;
			this->closed = false;
			this->numOfValues = 0;
			this->index = 0;
			this->root = true;
		}

		__device__ ScoreTree(double score, int * values, int valuelength) {
			this->score = score;
			this->children = new ScoreTree[CHILDREN_CAPACITY];
			nStoredValues = valuelength;
			if (storedValues != nullptr) { delete[] storedValues; }
			storedValues = new int[nStoredValues];
			for (int i = 0; i < nStoredValues; i++) {
				storedValues[i] = values[i];
			}
			if (nStoredValues <= 1) this->closed = true;
			else this->closed = false;
			this->numOfValues = valuelength;
			this->root = false;
		}

		__device__ bool fine() {
			if (root && nChildren > 0) {
				int i = children[nChildren - 1].index;
				if (i >= LOWER_BOUND && i <= UPPER_BOUND) {
					return isFirstUnique();
				}
			}
			return false;
		}

		__device__ bool isFirstUnique() {
			ScoreTree * tree = this;
			while (tree->nChildren > 0) {
				tree = &(tree->children[0]);
			}
			return tree->closed;
		}

		__device__ void addValues(double score, int * values, int nValues) {
			addValues(score, values, nValues, 0);
		}

		__device__ void print(int intent) {
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

			for (int i = 0; i < nChildren; i++) {
				children[i].print(intent);
			}
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
		ScoreTree * children;
		int nChildren;
		int capacityChildren;

		__device__ void addValues(double score, int * values, int& nValues, int counter) {
			for (int i = 0; i < nChildren; i++) {
				children[i].addValues(score, values, nValues, 0);
			}

			int * touched_end;
			int * untouched_end;
			int * values_end;
			int touched[N_SAMPLES];
			int untouched[N_SAMPLES];
			int nTouched = 0;
			int nUntouched = 0;

			if (!root) {
				touched_end = thrust::set_intersection(thrust::device, storedValues, storedValues + nStoredValues, values, values + nValues, touched);
				nTouched = thrust::distance(&touched[0], touched_end);
				untouched_end = thrust::set_difference(thrust::device, storedValues, storedValues + nStoredValues, values, values + nValues, untouched);
				nUntouched = thrust::distance(&untouched[0], untouched_end);
				values_end = thrust::set_difference(thrust::device, values, values + nValues, touched, touched_end, values);
				nValues = thrust::distance(values, values_end);
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
				for (int i = 0; i < nChildren; i++) {
					if (children[i].closed == false) {
						c = false;
						break;
					}
				}
				closed = c;
			}
		}

		__device__ ScoreTree addChild(double score, int * values, int valuelength, int childIndex) {
			if (nChildren > CHILDREN_CAPACITY) {
				printf("Set bigger children capacity");
			}
			ScoreTree child = ScoreTree(score, values, valuelength);
			child.index = childIndex;
			children[nChildren] = child;
			nChildren++;
			return child;
		}
};

#endif // SCORETREE_H

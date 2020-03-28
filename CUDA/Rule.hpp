#ifndef RULE_H
#define RULE_H
#define UNSEEN_NEGATIVE_EXAMPLES 5

#include "cuda_runtime.h"

enum Ruletype {XYRule, XRule, YRule, None};

class Rule
{
    public:
		__host__ Rule() {}
		__host__ Rule(int h_predicted, int h_correctlyPredicted, double confidence) {
			cudaMallocManaged(&predicted, sizeof(int));
			*predicted = h_predicted;
			cudaMallocManaged(&correctlyPredicted, sizeof(int));
			*correctlyPredicted = h_correctlyPredicted;
			cudaMallocManaged(&headrelation, sizeof(int));
			cudaMallocManaged(&headconstant, sizeof(int));
			cudaMallocManaged(&bodyconstantId, sizeof(int));
			*headrelation = INT_MAX;
			*headconstant = INT_MAX;
			*bodyconstantId = INT_MAX;
		}

		__host__ __device__ void print() {
			printf("%d %d %d %d\n", type, *headrelation, *headconstant, rulelength);
			for (int i = 0; i < rulelength; i++) {
				printf("%d %d\n", relationsFwd[i], relationsBwd[i]);
			}
			printf("%d\n", *bodyconstantId);
		}

		//Setter
		__host__ void setRuletype(Ruletype type) {
			this->type = type;
		}
		__host__ void setRulelength(int rulelength) {
			this->rulelength = rulelength;
		}
		__host__ void setHeadrelation(int * relation) {
			*headrelation = *relation;
		}
		__host__ void setRelationsFwd(int * bodyrelations) {
			cudaMallocManaged(&relationsFwd, this->rulelength * sizeof(int));
			for (int i = 0; i < this->rulelength; i++) {
				relationsFwd[i] = bodyrelations[i];
			}
			delete[] bodyrelations;
		}
		__host__ void setRelationsBwd(int * bodyrelations) {
			cudaMallocManaged(&relationsBwd, this->rulelength * sizeof(int));
			for (int i = 0; i < this->rulelength; i++) {
				relationsBwd[i] = bodyrelations[i];
			}
			delete[] bodyrelations;
		}
		__host__ void setHeadconstant(int * constant) {
			*headconstant = *constant;
		}
		__host__ void setBodyconstantId(int * id) {
			*bodyconstantId = *id;
		}


		//Getter
		__host__ __device__ Ruletype getRuletype() {
			return type;
		}
		__host__ __device__ int& getRulelength() {
			return rulelength;
		}
		__host__ __device__ int* getHeadrelation() {
			return headrelation;
		}
		__host__ __device__ int* getRelationsFwd() {
			return relationsFwd;
		}
		__host__ __device__ int* getRelationsBwd() {
			return relationsBwd;
		}
		__host__ __device__ int* getBodyconstantId() {
			return bodyconstantId;
		}
		__host__ __device__ int* getHeadconstant() {
			return headconstant;
		}

		__host__ __device__ double getAppliedConfidence() {
			return (double)*correctlyPredicted / ((double)*predicted + (double)UNSEEN_NEGATIVE_EXAMPLES);
		}

    protected:

    private:
        //Type of rule XYRule, XRule, YRule
        Ruletype type;
        //Length of body
        int rulelength;
        int * predicted;
        int * correctlyPredicted;
        double * confidence;
        // Relation of the head predicate
		int * headrelation;
        // Array with body relations (forward)
        int * relationsFwd;
		// Array with body relations (backward)
		int * relationsBwd;
        // For X or YRules contains constant of last atom, if present
        int * bodyconstantId;
        // For X or YRules contains the constant of the head predicate
		int * headconstant;
};

#endif // RULE_H

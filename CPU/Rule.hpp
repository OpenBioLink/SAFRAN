#ifndef RULE_H
#define RULE_H

#include "stdio.h"
#include "Properties.hpp"

enum Ruletype {XYRule, XRule, YRule, None};

class Rule
{
    public:
		Rule() {}
		Rule(int no1, int no2, double confidence) {
			this->predicted = no1;
			this->correctlyPredicted = no2;
			this->confidence = confidence;
			headrelation = nullptr;
			headconstant = nullptr;
			bodyconstantId = nullptr;
		}

		void print() {
			printf("\n%d %d %d %d\n", type, *headrelation, rulelength);
			for (int i = 0; i < rulelength; i++) {
				printf("%d %d\n", relationsFwd[i], relationsBwd[i]);
			}
		}

		//Setter
		void setRuletype(Ruletype type) {
			this->type = type;
		}
		void setRulelength(int rulelength) {
			this->rulelength = rulelength;
		}
		void setHeadrelation(int * relation) {
			headrelation = new int(*relation);
		}
		void setRelationsFwd(int * bodyrelations) {
			relationsFwd = bodyrelations;
		}
		void setRelationsBwd(int * bodyrelations) {
			relationsBwd = bodyrelations;
		}
		void setHeadconstant(int * constant) {
			headconstant = new int(*constant);
		}
		void setBodyconstantId(int * id) {
			bodyconstantId = new int(*id);
		}


		//Getter
		Ruletype getRuletype() {
			return type;
		}
		int& getRulelength() {
			return rulelength;
		}
		int* getHeadrelation() {
			return headrelation;
		}
		int* getRelationsFwd() {
			return relationsFwd;
		}
		int* getRelationsBwd() {
			return relationsBwd;
		}
		int* getBodyconstantId() {
			return bodyconstantId;
		}
		int* getHeadconstant() {
			return headconstant;
		}
		double getAppliedConfidence() {
			return (double)correctlyPredicted / ((double)predicted + (double)Properties::get().UNSEEN_NEGATIVE_EXAMPLES);
		}

		Rule& operator=(Rule* other)
		{
			type = other->type;
			rulelength = other->rulelength;
			predicted = other->predicted;
			confidence = other->confidence;
			correctlyPredicted = other->correctlyPredicted;
			headrelation = other->headrelation;
			relationsFwd = other->relationsFwd;
			relationsBwd = other->relationsBwd;
			bodyconstantId = other->bodyconstantId;
			headconstant = other->headconstant;
			return *this;
		}
    protected:

    private:
        //Type of rule XYRule, XRule, YRule
        Ruletype type;
        //Length of body
        int rulelength;
        int predicted;
        int correctlyPredicted;
        double confidence;
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

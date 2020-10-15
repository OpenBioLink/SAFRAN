#ifndef RULE_H
#define RULE_H

#include "stdio.h"
#include "Properties.hpp"
#include <vector>

enum Ruletype {XYRule, XRule, YRule, None};

class Rule
{
    public:
		Rule() {}
		Rule(int no1, int no2, double confidence) {
			this->predicted = no1;
			this->correctlyPredicted = no2;
			this->confidence = confidence;
			this->bodyhash = 0;
			headrelation = nullptr;
			headconstant = nullptr;
			bodyconstantId = nullptr;
			this->applied_confidence = (double)correctlyPredicted / ((double)predicted + (double)Properties::get().UNSEEN_NEGATIVE_EXAMPLES);
		}

		void print() {
			printf("\n%d %d %d\n", type, *headrelation, rulelength);
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
		void setRulestring(std::string rule) {
			rulestring = rule;
		}
		void setBuffer(std::vector<int> buffer) {
			this->buffer = buffer;
			buffered = true;
		}

		bool isBuffered() {
			return buffered;
		}

		bool is_c() {
			if(this->type == Ruletype::XYRule){
				return true;
			}
			return false;
		}

		bool is_ac1() {
			if ((this->type == Ruletype::XRule || this->type == Ruletype::YRule) && this->bodyconstantId == nullptr) {
				return true;
			}
			return false;
		}

		bool is_ac2() {
			if ((this->type == Ruletype::XRule || this->type == Ruletype::YRule) && this->bodyconstantId != nullptr) {
				return true;
			}
			return false;
		}

		void add_head_exception(int val) {
			head_exceptions.insert(val);
		}

		void add_tail_exception(int val) {
			tail_exceptions.insert(val);
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
			return applied_confidence;
		}
		std::string getRulestring() {
			return rulestring;
		}
		std::vector<int>& getBuffer() {
			return buffer;
		}
		void removeBuffer() {
			buffer.clear();
			buffered = false;
		}

		long long get_body_hash() {
			return bodyhash;
		}

		void compute_body_hash() {
			int* relations_this;
			if (this->type == Ruletype::YRule) {
				relations_this = this->relationsBwd;
			}
			else {
				relations_this = this->relationsFwd;
			}
			for (int i = 0; i < this->rulelength; i++) {
				bodyhash = bodyhash + pow((Properties::get().REL_SIZE + 1) * 2, i) * relations_this[i];
			}
		}

		bool is_body_equal(Rule other) {
			if (other.rulelength != this->rulelength) {
				return false;
			}
			else {
				int* relations_this;
				int* relations_that;

				if (this->type == Ruletype::YRule) {
					relations_this = this->relationsBwd;
				}
				else {
					relations_this = this->relationsFwd;
				}

				if (other.type == Ruletype::YRule) {
					relations_that = other.relationsBwd;
				}
				else {
					relations_that = other.relationsFwd;
				}

				for (int i = 0; i < this->rulelength; i++) {
					if (relations_this[i] != relations_that[i]) {
						return false;
					}
				}
				return true;
			}
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
			rulestring = other->rulestring;
			applied_confidence = other->applied_confidence;
			bodyhash = other->bodyhash;
			return *this;
		}

		std::unordered_set<int> head_exceptions;
		std::unordered_set<int> tail_exceptions;
    protected:

    private:
        //Type of rule XYRule, XRule, YRule
        Ruletype type;
        //Length of body
        int rulelength;
        int predicted;
        int correctlyPredicted;
        double confidence;
		double applied_confidence;
		long long bodyhash;
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

		std::string rulestring;

		std::vector<int> buffer;
		bool buffered = false;

};

#endif // RULE_H

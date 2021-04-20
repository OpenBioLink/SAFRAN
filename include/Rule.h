#ifndef RULE_H
#define RULE_H

#include "stdio.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <optional>

#include "Properties.hpp"

enum Ruletype { XYRule, XRule, YRule, None };

class Rule
{
public:
	Rule() {}
	Rule(int no1, int no2, double confidence);

	void print();

	//Setter
	void setID(int ID);
	void setRuletype(Ruletype type);
	void setRulelength(int rulelength);
	void setHeadrelation(int* relation);
	void setRelationsFwd(int* bodyrelations);
	void setRelationsBwd(int* bodyrelations);
	void setHeadconstant(int* constant);
	void setBodyconstantId(int* id);
	void setRulestring(std::string rule);

	bool isTrivial();

	void setBuffer(std::vector<int> buffer);
	bool isBuffered();
	std::vector<int>& getBuffer();
	void removeBuffer();

	void setHeadBuffer(int head, std::vector<int> buffer);
	bool isHeadBuffered(int head);
	std::vector<int>& getHeadBuffered(int head);
	void clearHeadBuffer();

	void setTailBuffer(int tail, std::vector<int> buffer);
	bool isTailBuffered(int tail);
	std::vector<int>& getTailBuffered(int tail);
	void clearTailBuffer();

	bool is_c();
	bool is_ac1();
	bool is_ac2();

	//Getter
	int& getID();
	Ruletype getRuletype();
	int& getRulelength();
	int* getHeadrelation();
	int* getRelationsFwd();
	int* getRelationsBwd();
	int* getBodyconstantId();
	int* getHeadconstant();
	double getAppliedConfidence();
	std::string getRulestring();
	long long get_body_hash();

	void compute_body_hash();
	bool is_body_equal(Rule other);
	Rule& operator=(Rule* other);

	/*
	std::unordered_set<int> head_exceptions;
	std::unordered_set<int> tail_exceptions;
	*/
protected:

private:
	//ID (Needed for explaination)
	int ID;
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
	int* headrelation;
	// Array with body relations (forward)
	int* relationsFwd;
	// Array with body relations (backward)
	int* relationsBwd;
	// For X || YRules contains constant of last atom, if present
	int* bodyconstantId;
	// For X || YRules contains the constant of the head predicate
	int* headconstant;

	std::string rulestring;

	std::optional<std::vector<int>> buffer{}; 
	bool buffered = false;

	std::optional<std::unordered_map<int, std::vector<int>>> tailBuffer{};
	std::optional<std::unordered_map<int, std::vector<int>>> headBuffer{};

	bool trivial = false;

};

#endif // RULE_H

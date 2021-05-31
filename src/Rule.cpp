#include "Rule.h"

Rule::Rule(int no1, int no2, double confidence) {
	this->predicted = no1;
	this->correctlyPredicted = no2;
	this->confidence = confidence;
	this->bodyhash = 0;
	headrelation = nullptr;
	headconstant = nullptr;
	bodyconstantId = nullptr;
	this->applied_confidence = (double)correctlyPredicted / ((double)predicted + (double)Properties::get().UNSEEN_NEGATIVE_EXAMPLES);
}

void Rule::print() {
	printf("\n%d %d %d\n", type, *headrelation, rulelength);
	for (int i = 0; i < rulelength; i++) {
		printf("%d %d\n", relationsFwd[i], relationsBwd[i]);
	}
}

//Setter
void Rule::setRuletype(Ruletype type) {
	this->type = type;
}
void Rule::setRulelength(int rulelength) {
	this->rulelength = rulelength;
}
void Rule::setHeadrelation(int * relation) {
	headrelation = new int(*relation);
}
void Rule::setRelationsFwd(int * bodyrelations) {
	relationsFwd = bodyrelations;
}
void Rule::setRelationsBwd(int * bodyrelations) {
	relationsBwd = bodyrelations;
}
void Rule::setHeadconstant(int * constant) {
	headconstant = new int(*constant);
}
void Rule::setBodyconstantId(int * id) {
	bodyconstantId = new int(*id);
}
void Rule::setRulestring(std::string rule) {
	rulestring = rule;
}

void Rule::setBuffer(std::vector<int> buffer) {
	this->buffer = buffer;
	buffered = true;
}
bool Rule::isBuffered() {
	return buffered;
}

void Rule::setHeadBuffer(int head, std::vector<int> buffer) {
	headBuffer[head] = buffer;
}
bool Rule::isHeadBuffered(int head) {
	return headBuffer.find(head) != headBuffer.end();
}
std::vector<int>& Rule::getHeadBuffered(int head) {
	return headBuffer[head];
}
void Rule::clearHeadBuffer() {
	headBuffer.clear();
}

void Rule::setTailBuffer(int tail, std::vector<int> buffer) {
	tailBuffer[tail] = buffer;
}
bool Rule::isTailBuffered(int tail) {
	return tailBuffer.find(tail) != tailBuffer.end();
}
std::vector<int>& Rule::getTailBuffered(int tail) {
	return tailBuffer[tail];
}
void Rule::clearTailBuffer() {
	tailBuffer.clear();
}

bool Rule::is_c() {
	if(this->type == Ruletype::XYRule){
		return true;
	}
	return false;
}

bool Rule::is_ac1() {
	if ((this->type == Ruletype::XRule || this->type == Ruletype::YRule) && this->bodyconstantId == nullptr) {
		return true;
	}
	return false;
}

bool Rule::is_ac2() {
	if ((this->type == Ruletype::XRule || this->type == Ruletype::YRule) && this->bodyconstantId != nullptr) {
		return true;
	}
	return false;
}

void Rule::add_head_exception(int val) {
	head_exceptions.insert(val);
}

void Rule::add_tail_exception(int val) {
	tail_exceptions.insert(val);
}

//Getter
Ruletype Rule::getRuletype() {
	return type;
}
int& Rule::getRulelength() {
	return rulelength;
}
int* Rule::getHeadrelation() {
	return headrelation;
}
int* Rule::getRelationsFwd() {
	return relationsFwd;
}
int* Rule::getRelationsBwd() {
	return relationsBwd;
}
int* Rule::getBodyconstantId() {
	return bodyconstantId;
}
int* Rule::getHeadconstant() {
	return headconstant;
}
double Rule::getAppliedConfidence() {
	return applied_confidence;
}
long long Rule::getCorrectlyPredicted() {
	return correctlyPredicted;
}
long long Rule::getPredicted() {
	return predicted + Properties::get().UNSEEN_NEGATIVE_EXAMPLES;
}
std::string Rule::getRulestring() {
	return rulestring;
}
std::vector<int>& Rule::getBuffer() {
	return buffer;
}
void Rule::removeBuffer() {
	buffer.clear();
	buffered = false;
}

long long Rule::get_body_hash() {
	return bodyhash;
}

void Rule::compute_body_hash() {
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

bool Rule::is_body_equal(Rule other) {
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

Rule& Rule::operator=(Rule* other)
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
	tail_exceptions = other->tail_exceptions;
	head_exceptions = other->head_exceptions;
	return *this;
}

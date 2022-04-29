#ifndef EXPL_H
#define EXPL_H

#include "Index.h"
#include "Rule.h"
#include "RuleReader.h"
#include "ClusteringReader.h"

class Explanation {
public:
	Explanation() {}
	virtual ~Explanation() {}

	virtual void begin() = 0;
	virtual void commit() = 0;
	virtual void begin_tr() = 0;
	virtual void commit_tr() = 0;
	virtual void insertEntities(Index* index) = 0;
	virtual void insertRelations(Index* index) = 0;
	virtual void insertRules(RuleReader* rr, int relsize, ClusteringReader* cr) = 0;

	virtual void insertTask(int prediction_id, bool is_head, int relation_id, int entity_id) = 0;
	virtual void insertPrediction(int task_id, int entity_id, bool hit, double confidence) = 0;
	virtual void insertRule_Entity(int rule_id, int task_id, int entity_id) = 0;

	virtual int getNextTaskID() = 0;
};

#endif //EXPL_H
#ifndef INMEMORY_EXPL_H
#define INMEMORY_EXPL_H

#include "Explanation.h"

class InMemoryExplanation: public Explanation {
public:
	InMemoryExplanation();
	~InMemoryExplanation();

	void begin();
	void commit();
	void begin_tr();
	void commit_tr();
	void insertEntities(Index* index);
	void insertRelations(Index* index);
	void insertRules(RuleReader* rr, int relsize, ClusteringReader* cr);

	void insertTask(int task_id, bool is_head, int relation_id, int entity_id);
	void insertPrediction(int task_id, int entity_id, bool hit, double confidence);
	void insertRule_Entity(int rule_id, int task_id, int entity_id);

	int getNextTaskID();

	std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, int>>> tripleBestRules;

private:
	int _task_id = 0;
	std::unordered_map<int, float> ruleConfidences;
	std::unordered_map<int, std::tuple<int, int, bool>> tasks;
	std::unordered_map<int, std::unordered_map<int, int>> taskEntityBestRules;
};

#endif //INMEMORY_EXPL_H

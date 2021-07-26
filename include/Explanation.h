#ifndef EXPL_H
#define EXPL_H

#include <sqlite3.h>
#include "Index.h"
#include "Rule.h"
#include "RuleReader.h"
#include "ClusteringReader.h"

class Explanation {
public:
	Explanation(std::string dbName, bool init = false);
	~Explanation();

	void begin();
	void commit();
	void begin_tr();
	void commit_tr();
	void insertEntities(Index* index);
	void insertRelations(Index* index);
	void insertRules(RuleReader* rr, int relsize, ClusteringReader* cr);

	void insertTask(int prediction_id, bool is_head, int relation_id, int entity_id);
	void insertPrediction(int task_id, int entity_id, bool hit, double confidence);
	void insertRule_Entity(int rule_id, int task_id, int entity_id);

	// OLD
	void insertCluster(int prediction_id, int entity_id, int cluster_id, double confidence);
	void insertRule_Cluster(int prediction_id, int entity_id, int cluster_id, int rule_id);

	int getNextTaskID();
private:
	sqlite3* db;
	int task_id = 0;
	void initDb();
	void checkErrorCode(int code);
	void checkErrorCode(int code, char* sql);
	sqlite3_stmt* prepare(char* sql);
	void finalize(sqlite3_stmt* stmt);
};

#endif //EXPL_H
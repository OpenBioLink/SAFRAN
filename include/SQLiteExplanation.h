#ifndef SQLITE_EXPL_H
#define SQLITE_EXPL_H

#include <sqlite3.h>
#include "Explanation.h"

class SQLiteExplanation: public Explanation {
public:
    SQLiteExplanation(std::string dbName, bool init = false);
    ~SQLiteExplanation();

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

    // OLD
    void insertCluster(int prediction_id, int entity_id, int cluster_id, double confidence);
    void insertRule_Cluster(int prediction_id, int entity_id, int cluster_id, int rule_id);

    int getNextTaskID();
private:
    sqlite3* db;
    int _task_id = 0;
    void initDb();
    void checkErrorCode(int code);
    void checkErrorCode(int code, char* sql);
    sqlite3_stmt* prepare(char* sql);
    void finalize(sqlite3_stmt* stmt);
};

#endif //SQLITE_EXPL_H
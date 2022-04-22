#include "SQLiteExplanation.h"

SQLiteExplanation::SQLiteExplanation(std::string dbName, bool init) {
    checkErrorCode(sqlite3_open_v2(dbName.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr));
    if (init) initDb();
}

SQLiteExplanation::~SQLiteExplanation() {
    sqlite3_close(db);
}

void SQLiteExplanation::initDb() {

    checkErrorCode(sqlite3_exec(db, "PRAGMA synchronous=OFF", NULL, NULL, NULL));
    checkErrorCode(sqlite3_exec(db, "PRAGMA count_changes=OFF", NULL, NULL, NULL));
    checkErrorCode(sqlite3_exec(db, "PRAGMA journal_mode=MEMORY", NULL, NULL, NULL));
    checkErrorCode(sqlite3_exec(db, "PRAGMA temp_store=MEMORY", NULL, NULL, NULL));

    char* sql;

    sql = "CREATE TABLE Entity("  \
        "ID INT PRIMARY KEY     NOT NULL," \
        "NAME           TEXT    NOT NULL" \
        ");";
    checkErrorCode(sqlite3_exec(db, sql, NULL, NULL, NULL), sql);

    sql = "CREATE TABLE Relation(" \
        "ID INT PRIMARY KEY     NOT NULL," \
        "NAME           TEXT    NOT NULL" \
        ");";
    checkErrorCode(sqlite3_exec(db, sql, NULL, NULL, NULL), sql);

    sql = "CREATE TABLE Rule(" \
        "ID INT PRIMARY KEY     NOT NULL," \
        "HEAD_CLUSTER_ID INT NOT NULL," \
        "TAIL_CLUSTER_ID INT NOT NULL," \
        "DEF           TEXT    NOT NULL," \
        "CONFIDENCE DOUBLE NOT NULL,"\
        "PREDICTED INT NOT NULL,"\
        "CORRECTLY_PREDICTED INT NOT NULL"\
        ");";
    checkErrorCode(sqlite3_exec(db, sql, NULL, NULL, NULL), sql);

    sql = "CREATE TABLE Task("  \
        "ID INT PRIMARY KEY     NOT NULL," \
        "IsHead BOOL," \
        "EntityID INT," \
        "RelationID  INT," \
        "FOREIGN KEY(EntityID) REFERENCES Entity(ID)," \
        "FOREIGN KEY(RelationID) REFERENCES Relation(ID)" \
        ");";
    checkErrorCode(sqlite3_exec(db, sql, NULL, NULL, NULL), sql);

    sql = "CREATE TABLE Prediction(" \
        "TaskID INT     NOT NULL," \
        "EntityID INT     NOT NULL," \
        "Hit BOOL NOT NULL,"
          "CONFIDENCE REAL NOT NULL,"\
        "FOREIGN KEY(TaskID) REFERENCES Task(ID)," \
        "FOREIGN KEY(EntityID) REFERENCES Entity(ID)," \
        "PRIMARY KEY(TaskID, EntityID)" \
        ");";
    checkErrorCode(sqlite3_exec(db, sql, NULL, NULL, NULL), sql);

    sql = "CREATE TABLE Rule_Entity(" \
        "RuleID INT NOT NULL," \
        "TaskID INT NOT NULL," \
        "EntityID INT NOT NULL," \
        "FOREIGN KEY(RuleID) REFERENCES Rule(ID)," \
        "FOREIGN KEY(TaskID) REFERENCES Task(ID)," \
        "FOREIGN KEY(EntityID) REFERENCES Entity(ID)," \
        "PRIMARY KEY(RuleID, TaskID, EntityID)" \
        ");";
    checkErrorCode(sqlite3_exec(db, sql, NULL, NULL, NULL), sql);

    sql = "CREATE INDEX asdf ON Rule_Entity (" \
        "TaskID," \
        "EntityID" \
        ");";
    checkErrorCode(sqlite3_exec(db, sql, NULL, NULL, NULL), sql);
}

void SQLiteExplanation::insertEntities(Index* index) {
    char* sql = "INSERT INTO Entity (ID,NAME) VALUES (?,?);";
    sqlite3_stmt* stmt = prepare(sql);
    for (int id = 0; id < index->getNodeSize(); id++) {

        // bind the value
        checkErrorCode(sqlite3_bind_int(stmt, 1, id));

        std::string curie = *index->getStringOfNodeId(id);
        checkErrorCode(sqlite3_bind_text(stmt, 2, curie.c_str(), strlen(curie.c_str()), 0));

        checkErrorCode(sqlite3_step(stmt));
        checkErrorCode(sqlite3_reset(stmt));
    }
    finalize(stmt);
}
void SQLiteExplanation::insertRelations(Index* index) {
    char* sql = "INSERT INTO Relation (ID,NAME) VALUES (?,?);";
    sqlite3_stmt* stmt = prepare(sql);
    for (int id = 0; id < index->getRelSize(); id++) {

        // bind the value
        checkErrorCode(sqlite3_bind_int(stmt, 1, id));

        std::string relation = *index->getStringOfRelId(id);
        checkErrorCode(sqlite3_bind_text(stmt, 2, relation.c_str(), strlen(relation.c_str()), 0));

        checkErrorCode(sqlite3_step(stmt));
        checkErrorCode(sqlite3_reset(stmt));
    }
    finalize(stmt);
}

void SQLiteExplanation::insertRules(RuleReader* rr, int relsize, ClusteringReader* cr) {
    int* adj_begin = rr->getCSR()->getAdjBegin();
    Rule* rules_adj_list = rr->getCSR()->getAdjList();

    char* sql = "INSERT INTO Rule (ID,HEAD_CLUSTER_ID,TAIL_CLUSTER_ID,DEF,CONFIDENCE,PREDICTED,CORRECTLY_PREDICTED) VALUES (?,?,?,?,?,?,?);";
    sqlite3_stmt* stmt = prepare(sql);

    if (cr == nullptr) {
        for (int rel = 0; rel < relsize; rel++) {
            int ind_ptr = adj_begin[3 + rel];
            int lenRules = adj_begin[3 + rel + 1] - ind_ptr;
            for (int i = 0; i < lenRules; i++) {

                Rule& r = rules_adj_list[ind_ptr + i];
                // bind the value
                checkErrorCode(sqlite3_bind_int(stmt, 1, r.getID()));

                checkErrorCode(sqlite3_bind_int(stmt, 2, 0));
                checkErrorCode(sqlite3_bind_int(stmt, 3, 0));

                std::string rulestr = r.getRulestring();
                checkErrorCode(sqlite3_bind_text(stmt, 4, rulestr.c_str(), strlen(rulestr.c_str()), 0));

                checkErrorCode(sqlite3_bind_double(stmt, 5, r.getAppliedConfidence()));
                checkErrorCode(sqlite3_bind_int(stmt, 6, r.getPredicted()));
                checkErrorCode(sqlite3_bind_int(stmt, 7, r.getCorrectlyPredicted()));

                checkErrorCode(sqlite3_step(stmt));
                checkErrorCode(sqlite3_reset(stmt));
            }
        }
    }
    else {
        std::unordered_map<int, std::pair<std::pair<bool, std::vector<std::vector<int>>>, std::pair<bool, std::vector<std::vector<int>>>>> rel2clusters = cr->getRelToClusters();
        for (int rel = 0; rel < relsize; rel++) {
            int ind_ptr = adj_begin[3 + rel];
            int lenRules = adj_begin[3 + rel + 1] - ind_ptr;

            std::unordered_map<int, int> ruleToHeadCluster;
            std::unordered_map<int, int> ruleToTailCluster;

            // Head clusters
            std::vector<std::vector<int>> cluster = rel2clusters[rel].first.second;
            for (int cluster_id = 0; cluster_id < cluster.size(); cluster_id++) {
                for (auto rule : cluster[cluster_id]) {
                    Rule& r = rules_adj_list[ind_ptr + rule];
                    ruleToHeadCluster[r.getID()] = cluster_id;
                }
            }

            cluster = rel2clusters[rel].second.second;
            for (int cluster_id = 0; cluster_id < cluster.size(); cluster_id++) {
                for (auto rule : cluster[cluster_id]) {
                    Rule& r = rules_adj_list[ind_ptr + rule];
                    ruleToTailCluster[r.getID()] = cluster_id;
                }
            }

            for (int i = 0; i < lenRules; i++) {

                Rule& r = rules_adj_list[ind_ptr + i];
                // bind the value
                checkErrorCode(sqlite3_bind_int(stmt, 1, r.getID()));

                checkErrorCode(sqlite3_bind_int(stmt, 2, ruleToHeadCluster[r.getID()]));

                checkErrorCode(sqlite3_bind_int(stmt, 3, ruleToTailCluster[r.getID()]));

                std::string rulestr = r.getRulestring();
                checkErrorCode(sqlite3_bind_text(stmt, 4, rulestr.c_str(), strlen(rulestr.c_str()), 0));

                checkErrorCode(sqlite3_bind_double(stmt, 5, r.getAppliedConfidence()));
                checkErrorCode(sqlite3_bind_int(stmt, 6, r.getPredicted()));
                checkErrorCode(sqlite3_bind_int(stmt, 7, r.getCorrectlyPredicted()));

                checkErrorCode(sqlite3_step(stmt));
                checkErrorCode(sqlite3_reset(stmt));
            }
        }
    }

    finalize(stmt);
}

void SQLiteExplanation::insertTask(int task_id, bool is_head, int relation_id, int entity_id) {
    //std::cout << "Task " << task_id << " " << is_head << " " << relation_id << " " << entity_id << "\n";
    char* sql = "INSERT INTO Task (ID,IsHead,RelationID,EntityId) VALUES (?,?,?,?);";
    sqlite3_stmt* stmt = prepare(sql);

    checkErrorCode(sqlite3_bind_int(stmt, 1, task_id), "insert task bind 1");
    checkErrorCode(sqlite3_bind_int(stmt, 2, (int)is_head), "insert task bind 2");
    checkErrorCode(sqlite3_bind_int(stmt, 3, relation_id), "insert task bind 3");
    checkErrorCode(sqlite3_bind_int(stmt, 4, entity_id), "insert task bind 4");

    checkErrorCode(sqlite3_step(stmt), "insert task step");
    finalize(stmt);
}

void SQLiteExplanation::insertPrediction(int task_id, int entity_id, bool hit, double confidence) {
    //std::cout << "Prediction " << task_id << " " << entity_id << " " << " " << confidence << "\n";
    char* sql = "INSERT INTO Prediction (TaskID,EntityID,Hit,Confidence) VALUES (?,?,?,?);";
    sqlite3_stmt* stmt = prepare(sql);

    checkErrorCode(sqlite3_bind_int(stmt, 1, task_id), "insert pred bind 1");
    checkErrorCode(sqlite3_bind_int(stmt, 2, entity_id), "insert pred bind 2");
    checkErrorCode(sqlite3_bind_double(stmt, 3, (int)hit), "insert pred bind 3");
    checkErrorCode(sqlite3_bind_double(stmt, 4, confidence), "insert pred bind 4");

    checkErrorCode(sqlite3_step(stmt), "insert pred step");
    finalize(stmt);
}

void SQLiteExplanation::insertCluster(int task_id, int entity_id, int cluster_id, double confidence) {
    //std::cout << "Cluster " << task_id << " " << entity_id << " " << cluster_id << " " << confidence << "\n";
    char* sql = "INSERT INTO Cluster (PredictionTaskID, PredictionEntityID, ID, Confidence) VALUES (?,?,?,?);";
    sqlite3_stmt* stmt = prepare(sql);

    checkErrorCode(sqlite3_bind_int(stmt, 1, task_id), "insert clus bind 1");
    checkErrorCode(sqlite3_bind_int(stmt, 2, entity_id), "insert clus bind 2");
    checkErrorCode(sqlite3_bind_int(stmt, 3, cluster_id), "insert clus bind 3");
    checkErrorCode(sqlite3_bind_double(stmt, 4, confidence), "insert clus bind 1");

    checkErrorCode(sqlite3_step(stmt), sql);
    finalize(stmt);
}

void SQLiteExplanation::insertRule_Cluster(int task_id, int entity_id, int cluster_id, int rule_id) {
    //std::cout << "Rule_Cluster " << task_id << " " << entity_id << " " << cluster_id << " " << rule_id << "\n";
    char* sql = "INSERT INTO Rule_Cluster (ClusterPredictionTaskID,ClusterPredictionEntityID,ClusterID,RuleID) VALUES (?,?,?,?);";
    sqlite3_stmt* stmt = prepare(sql);

    checkErrorCode(sqlite3_bind_int(stmt, 1, task_id));
    checkErrorCode(sqlite3_bind_int(stmt, 2, entity_id));
    checkErrorCode(sqlite3_bind_int(stmt, 3, cluster_id));
    checkErrorCode(sqlite3_bind_int(stmt, 4, rule_id));

    checkErrorCode(sqlite3_step(stmt));
    finalize(stmt);
}

void SQLiteExplanation::insertRule_Entity(int rule_id, int task_id, int entity_id) {
    //std::cout << "Rule_Cluster " << task_id << " " << entity_id << " " << cluster_id << " " << rule_id << "\n";
    char* sql = "INSERT OR IGNORE INTO Rule_Entity (RuleID, TaskID, EntityID) VALUES (?,?,?);";
    sqlite3_stmt* stmt = prepare(sql);

    checkErrorCode(sqlite3_bind_int(stmt, 1, rule_id));
    checkErrorCode(sqlite3_bind_int(stmt, 2, task_id));
    checkErrorCode(sqlite3_bind_int(stmt, 3, entity_id));

    checkErrorCode(sqlite3_step(stmt));
    finalize(stmt);
}

void SQLiteExplanation::begin_tr() {
    checkErrorCode(sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL), "BEGIN TRANSACTION");
}

void SQLiteExplanation::begin() {
    //sqlite3_mutex_enter(sqlite3_db_mutex(db));
}

sqlite3_stmt* SQLiteExplanation::prepare(char* sql) {
    sqlite3_stmt* stmt;
    checkErrorCode(sqlite3_prepare(db, sql, -1, &stmt, NULL), sql);
    return stmt;
}

void SQLiteExplanation::finalize(sqlite3_stmt* stmt) {
    checkErrorCode(sqlite3_finalize(stmt));
}

void SQLiteExplanation::commit() {
    //sqlite3_mutex_leave(sqlite3_db_mutex(db));
}

void SQLiteExplanation::commit_tr() {
    checkErrorCode(sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL), "COMMIT TRANSACTION");
}

void SQLiteExplanation::checkErrorCode(int code) {
    if (code != SQLITE_OK && code != SQLITE_DONE) {
        const char* err;
        if (this->db) {
            err = sqlite3_errmsg(this->db);
        }
        else {
            err = sqlite3_errstr(code);
        }
        std::cerr << "Error " << code << ": " << err << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void SQLiteExplanation::checkErrorCode(int code, char* sql) {
    if (code != SQLITE_OK && code != SQLITE_DONE) {
        const char* err;
        if (this->db) {
            err = sqlite3_errmsg(this->db);
        }
        else {
            err = sqlite3_errstr(code);
        }
        std::cerr << sql << "\n";
        std::cerr << "Error " << code << ": " << err << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int SQLiteExplanation::getNextTaskID() {
    int task_id_;
#pragma omp critical
    {
        _task_id++;
        task_id_ = _task_id;
    }
    return task_id_;
}

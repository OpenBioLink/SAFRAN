#include "InMemoryExplanation.h"

InMemoryExplanation::InMemoryExplanation() {
}

InMemoryExplanation::~InMemoryExplanation() = default;

void InMemoryExplanation::insertEntities(Index* index) {
}

void InMemoryExplanation::insertRelations(Index* index) {
}

void InMemoryExplanation::insertRules(RuleReader* rr, int relsize, ClusteringReader* cr) {
    int* adj_begin = rr->getCSR()->getAdjBegin();
    Rule* rules_adj_list = rr->getCSR()->getAdjList();

    if (cr == nullptr) {
        for (int rel = 0; rel < relsize; rel++) {
            int ind_ptr = adj_begin[3 + rel];
            int lenRules = adj_begin[3 + rel + 1] - ind_ptr;
            for (int i = 0; i < lenRules; i++) {
                Rule &r = rules_adj_list[ind_ptr + i];
                int rule_id = r.getID();
                auto conf = (float)r.getAppliedConfidence();
                ruleConfidences[rule_id] = conf;
            }
        }
    } else {
        // Not implemented yet.
        return;
    }
}

void InMemoryExplanation::insertTask(int task_id, bool is_head, int relation_id, int entity_id) {
#pragma omp critical
    {
        tasks[task_id] = {relation_id, entity_id, is_head};
    }
}

void InMemoryExplanation::insertPrediction(int task_id, int entity_id, bool hit, double confidence) {
}

void InMemoryExplanation::insertRule_Entity(int rule_id, int task_id, int other_entity_id) {
    bool need_update = true;
    if (taskEntityBestRules.find(task_id) != taskEntityBestRules.end()) {
        if (taskEntityBestRules[task_id].find(other_entity_id) != taskEntityBestRules[task_id].end()) {
            int prev_rule_id = taskEntityBestRules[task_id][other_entity_id];
            float prev_conf = ruleConfidences[prev_rule_id], new_conf = ruleConfidences[rule_id];
            if (new_conf <= prev_conf) {
                need_update = false;
            }
        }
    }
    if (need_update) {
        auto & p = tasks[task_id];
        int relation_id = std::get<0>(p), entity_id = std::get<1>(p), is_head = std::get<2>(p);
        int head_id, tail_id;
        if (is_head) {
            head_id = entity_id;
            tail_id = other_entity_id;
        } else {
            head_id = other_entity_id;
            tail_id = entity_id;
        }
#pragma omp critical
        // Update the current best rule, according to the "applymax" action
        {
            taskEntityBestRules[task_id][other_entity_id] = rule_id;
            tripleBestRules[head_id][relation_id][tail_id] = rule_id;
        }
    }
}

void InMemoryExplanation::begin_tr() {
}

void InMemoryExplanation::begin() {
}

void InMemoryExplanation::commit() {
}

void InMemoryExplanation::commit_tr() {
}

int InMemoryExplanation::getNextTaskID() {
    int task_id_;
#pragma omp critical
    {
        _task_id++;
        task_id_ = _task_id;
    }
    return task_id_;
}
#include "safran_wrapper.h"

pysafran::pysafran(std::string train_path, std::string rule_path, int num_threads) {
    // TODO: initialize these globally, or make those a part of pysafran instance.
    Properties::get().VERBOSE = 0;
    Properties::get().PREDICT_UNKNOWN = 1;
    if (num_threads != -1) {
        omp_set_num_threads(num_threads);
    }

    this->index = new Index();
    index->addNode(Properties::get().REFLEXIV_TOKEN);
    index->addNode(Properties::get().UNK_TOKEN);

    this->graph = new TraintripleReader(train_path, index);
    Properties::get().REL_SIZE = index->getRelSize();

    this->rr = new RuleReader(rule_path, index, graph);
    this->vtr = new ValidationtripleReader("/dev/null", index, graph);

    this->exp = new InMemoryExplanation();
    this->exp->insertRules(rr, index->getRelSize(), nullptr);

    this->ra = new RuleApplication(index, graph, vtr, rr, exp);
}

pysafran::~pysafran() {
    delete this->ra;
    delete this->vtr;
    delete this->rr;
    delete this->graph;
    delete this->index;
    delete this->exp;
}

std::vector<std::pair<std::string, std::pair<std::string, std::pair<std::string, std::pair<float, int>>>>> pysafran::query(const std::string & action, size_t k,
                                                                                                                           const std::vector<std::string> & flat_triples) const {
    // "Read" triples
    if (Properties::get().VERBOSE == 1) {
        std::cout << "read start" << std::endl;
    }
    TesttripleReader ttr(index, graph, 0);
    std::vector<std::tuple<std::string, std::string, std::string>> triples;
    size_t i = 0;
    while (i < flat_triples.size()) {
        triples.emplace_back(flat_triples[i], flat_triples[i + 1], flat_triples[i + 2]);
        i += 3;
    }
    ttr.read(triples);
    if (Properties::get().VERBOSE == 1) {
        std::cout << "read finish" << std::endl;
    }

    if (Properties::get().VERBOSE == 1) {
        std::cout << "apply start" << std::endl;
    }
    // Apply specific rule application
    std::vector<std::tuple<int, int, int, float50>> outAction;
    if (action == "applymax") {
        if (Properties::get().VERBOSE == 1) {
            std::cout << "ra start" << std::endl;
        }
        ra->updateTTR(&ttr);
        if (Properties::get().VERBOSE == 1) {
            std::cout << "ra end" << std::endl;
        }
        outAction = ra->apply_only_max_in_memory(k);
    }
    if (Properties::get().VERBOSE == 1) {
        std::cout << "apply finish" << std::endl;
    }

    // Transform node ids to node names and only retain top-k
    std::vector<std::pair<std::string, std::pair<std::string, std::pair<std::string, std::pair<float, int>>>>> out;
    if (Properties::get().VERBOSE == 1) {
        std::cout << "transform start" << std::endl;
    }
    for (auto & p : outAction) {
        int head_id = std::get<0>(p), relation_id = std::get<1>(p), tail_id = std::get<2>(p);
        float val = (float)std::get<3>(p);
        std::string headStr = *this->index->getStringOfNodeId(head_id);
        std::string predStr = *this->index->getStringOfRelId(relation_id);
        std::string tailStr = *this->index->getStringOfNodeId(tail_id);
        int rule_id = exp->tripleBestRules[head_id][relation_id][tail_id];
        std::pair<float, int> p1 = {val, rule_id};
        std::pair<std::string, std::pair<float, int>> p2 = {tailStr, p1};
        std::pair<std::string, std::pair<std::string, std::pair<float, int>>> p3 = {predStr, p2};
        out.emplace_back(headStr, p3);
    }
    if (Properties::get().VERBOSE == 1) {
        std::cout << "transform finish" << std::endl;
    }
    return out;
}

#include "ClusteringEngine.h"

ClusteringEngine::ClusteringEngine(Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->index = index;
	this->graph = graph;
	this->ttr = ttr;
	this->vtr = vtr;
	this->rr = rr;
	adj_lists = graph->getCSR()->getAdjList();
	adj_list_starts = graph->getCSR()->getAdjBegin();
	vt_adj_lists = vtr->getCSR()->getAdjList();
	vt_adj_list_starts = vtr->getCSR()->getAdjBegin();
	rules_adj_list = rr->getCSR()->getAdjList();
	adj_begin = rr->getCSR()->getAdjBegin();
	WORKER_THREADS = Properties::get().WORKER_THREADS;
}

void ClusteringEngine::learn() {
	fopen_s(&pFile, Properties::get().PATH_CLUSTER.c_str(), "w");

	std::vector<int> relations;
	if(Properties::get().REL_IDS.size() == 0){
		for(auto kv : vtr->getRelHeadToTails()) {
    			relations.push_back(kv.first);
		} 
	} else {
		relations = Properties::get().REL_IDS;
	}
	std::sort(relations.begin(), relations.end());

	int rellen = relations.size();
	auto relvalid = relations.begin();
	while(relvalid != relations.end()){
		int i = *relvalid;
		int ind_ptr = adj_begin[3 + i];
		int len = adj_begin[3 + i + 1] - ind_ptr;
		std::cout << "Calculating cluster for " << *index->getStringOfRelId(i) << " " << len << "\n";

		Clustering* cluster = new Clustering(i, len, index, graph, ttr, vtr, rr);

		std::string jacc_file = Properties::get().PATH_JACCARD + std::string("/") + std::to_string(i) + std::string("_jacc.bin");
		auto result = cluster->learn_cluster(jacc_file);

		writeThresh(result);

		delete cluster;
		std::cout << "Cluster calculated for " << i << "/" << rellen << " rule relations" << std::endl;
		relvalid++;
	}

	std::cout << "DONEZO" << std::endl;

	fclose(pFile);
}

void ClusteringEngine::writeThresh(std::string result) {
	fprintf(pFile, "%s", result.c_str());
	fflush(pFile);
}

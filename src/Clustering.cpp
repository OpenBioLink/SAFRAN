#include "Clustering.h"

Clustering::Clustering(int relation, int size, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->strat = Properties::get().STRATEGY;
	this->portions = Properties::get().PORTIONS;
	this->index = index;
	this->graph = graph;
	this->ttr = ttr;
	this->vtr = vtr;
	this->rr = rr;
	adj_begin = rr->getCSR()->getAdjBegin();
	rules_adj_list = rr->getCSR()->getAdjList();
	ind_ptr = adj_begin[3 + relation];
	lenRules = adj_begin[3 + relation + 1] - ind_ptr;
	this->relation = relation;
	samples_size = size;
	features_size = size;

	WORKER_THREADS = Properties::get().WORKER_THREADS;
}

std::string Clustering::learn_cluster(std::string jacc_path) {
	std::vector<std::pair<int, double>>* jacc = read_jaccard(jacc_path);
	Graph* g = new Graph(samples_size, jacc, rules_adj_list, ind_ptr);
	std::cout << "Calced jaccs\n";

	const unsigned long long MAX_BUF = Properties::get().BUFFER_SIZE;
	long long CURR_BUF = 0;
	bool stop = false;
	int asdf = 0;

	int* vt_adj_list = vtr->getCSR()->getAdjList();
	int* vt_adj_begin = vtr->getCSR()->getAdjBegin();

	RuleGraph* rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < lenRules; i++) {
		if (lenRules > 100 and (i % ((lenRules - 1) / 100)) == 0) {
			util::printProgress((double)i / (double)(lenRules - 1));
		}
		Rule& currRule = rules_adj_list[ind_ptr + i];
		if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::XRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac2() and currRule.getRuletype() == Ruletype::YRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::XRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac1() and currRule.getRuletype() == Ruletype::YRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, results_vec, false, true);
			currRule.setBuffer(results_vec);
		}

		if (currRule.is_c()) {
			if (stop) { 
				continue; 
			} else {
				#pragma omp atomic
				asdf++;
			}

			{
				int* v_adj_list = &(vt_adj_list[vt_adj_begin[relation * 2]]);
				if (vtr->getRelHeadToTails().find(relation) == vtr->getRelHeadToTails().end()) continue;
				for (auto heads = vtr->getRelHeadToTails()[relation].begin(); heads != vtr->getRelHeadToTails()[relation].end(); heads++) {
					int head = heads->first;
					int lenTails = heads->second.size();
					if (lenTails > 0) {
						std::vector<int> tailresults_vec;
						rulegraph->searchDFSSingleStart_filt(true, head, head, currRule, false, tailresults_vec, false, true);
						if (tailresults_vec.size() + CURR_BUF < MAX_BUF) {
#pragma omp critical
							{
								if (tailresults_vec.size() + CURR_BUF < MAX_BUF) {
									currRule.setHeadBuffer(head, tailresults_vec);
									CURR_BUF = CURR_BUF + tailresults_vec.size();
								}
								else {
									stop = true;
								}
							}
						}
						else {
							stop = true;
						}
						if (stop) break;
					}
				}
			}

			{

				int* v_adj_list = &(vt_adj_list[vt_adj_begin[relation * 2 + 1]]);
				for (auto tails = vtr->getRelTailToHeads()[relation].begin(); tails != vtr->getRelTailToHeads()[relation].end(); tails++) {
					int tail = tails->first;
					int lenHeads = tails->second.size();
					if (lenHeads > 0) {
						std::vector<int> headresults_vec;
						rulegraph->searchDFSSingleStart_filt(false, tail, tail, currRule, true, headresults_vec, false, true);
						if (headresults_vec.size() + CURR_BUF < MAX_BUF) {
#pragma omp critical
							{
								if (headresults_vec.size() + CURR_BUF < MAX_BUF) {
									currRule.setTailBuffer(tail, headresults_vec);
									CURR_BUF = CURR_BUF + headresults_vec.size();
								}
								else {
									stop = true;
								}
							}
						}
						else {
							stop = true;
						}
						if (stop) break;
					}
				}	
			}
		}
	}
	std::cout << "DONE buffering " << CURR_BUF << "\n";

	learn_parameters(g, rulegraph);

	delete rulegraph;
	for (int i = 0; i < lenRules; i++) {
		Rule& currRule = rules_adj_list[ind_ptr + i];
		if (currRule.isBuffered()) {
			currRule.removeBuffer();
		}
		if (currRule.is_c()) {
			currRule.clearHeadBuffer();
			currRule.clearTailBuffer();
		}
	}

	std::cout << "Calced params\n";

	std::cout << "MAX " << max_c_c << " " << max_ac1_ac1 << " " << max_ac2_ac2 << " " << max_c_ac2 << " " << max_c_ac1 << " " << max_ac1_ac2 << " " << max_mrr << "\n";
	std::ostringstream stringStream;
	stringStream << "Relation\t" << *index->getStringOfRelId(relation) << "\t" << max_c_c << " " << max_ac1_ac1 << " " << max_ac2_ac2 << " " << max_c_ac2 << " " << max_c_ac1 << " " << max_ac1_ac2 << " " << max_mrr << "\n";
	for (auto cluster : max_cluster) {
		for (auto rule : cluster) {
			Rule& r = rules_adj_list[ind_ptr + rule];
			stringStream << r.getRulestring() << "\t";
		}
		stringStream << "\n";
	}
	stringStream << "\n";
	delete[] jacc;
	delete g;

	return stringStream.str();
}

std::vector<std::pair<int, double>>* Clustering::read_jaccard(std::string path) {
	int size = -1;
	std::vector<std::pair<int, double>>* jaccs_bin;

	std::ifstream myfile(path, std::ios::binary);

	if (myfile.is_open()) {
		int size;
		myfile.read((char*)(&size), sizeof size);
		jaccs_bin = new std::vector<std::pair<int, double>>[size];
		for (int i = 0; i < size; i++) {
			int len;
			myfile.read((char*)(&len), sizeof len);
			for (int j = 0; j < len; j++) {
				int r;
				double jacc;
				myfile.read((char*)(&r), sizeof r);
				myfile.read((char*)(&jacc), sizeof jacc);
				jaccs_bin[i].push_back(std::make_pair(r, jacc));
			}
		}
		myfile.close();
		return jaccs_bin;
	}
	else {
		std::cout << "Unable to open rule file " << path << std::endl;
		exit(-1);
	}
}

void Clustering::learn_parameters(Graph * g, RuleGraph * rulegraph) {

	int iterations;
	if (strat.compare("gridsingle") == 0) {
		iterations = portions + 1;
	}
	else if (strat.compare("random") == 0) {
		iterations = Properties::get().ITERATIONS;
	}
	else {
		throw std::runtime_error("Strategy not supported");
	}


#pragma omp parallel for schedule(dynamic)
	for(int i = 0; i < iterations; i++){
		if ((i % ((iterations - 1) / 100)) == 0) {
			util::printProgress((double)i / (double)(iterations - 1));
		}

		ApplicationEngine* noe = new ApplicationEngine(relation, rulegraph, index, graph, ttr, vtr, rr);

		if (i == 0) {
			std::vector<std::vector<int>> clusters;
			std::vector<int> cluster;
			for (int i = 0; i < samples_size; i++) {
				Rule& r = rules_adj_list[ind_ptr + i];
				cluster.push_back(i);
			}
			clusters.push_back(cluster);
			
			auto result = noe->max(clusters);

			if (result > max_mrr) {
#pragma omp critical
				if (result > max_mrr) {
					max_mrr = result;
					max_c_c = 0.0;
					max_ac1_ac1 = 0.0;
					max_ac2_ac2 = 0.0;
					max_c_ac2 = 0.0;
					max_c_ac1 = 0.0;
					max_ac1_ac2 = 0.0;
					max_cluster = clusters;
				}
			}
			delete noe;
		} else {
			double c_c, ac1_ac1, ac2_ac2, c_ac2, c_ac1, ac1_ac2;
			if(strat.compare("gridsingle")==0){
				double thresh = (double)i / portions;
				c_c = thresh;
				ac1_ac1 = thresh;
				ac2_ac2 = thresh;
				c_ac2 = thresh;
				c_ac1 = thresh;
				ac1_ac2 = thresh;
			}
			else if (strat.compare("random")==0) {
				portions = 10;
				std::mt19937 a = util::get_prng();
				c_c = (double)(a() % (portions + 1)) / portions;
				ac1_ac1 = (double)(a() % (portions + 1)) / portions;
				ac2_ac2 = (double)(a() % (portions + 1)) / portions;
				c_ac2 = (double)(a() % (portions + 1)) / portions;
				c_ac1 = (double)(a() % (portions + 1)) / portions;
				ac1_ac2 = (double)(a() % (portions + 1)) / portions;
			}


			ApplicationEngine* noe = new ApplicationEngine(relation, rulegraph, index, graph, ttr, vtr, rr);
			std::vector<std::vector<int>> clusters;

			bool* visited = new bool[samples_size];

			for (int i = 0; i < samples_size; i++) {
				visited[i] = false;
			}

			for (int i = 0; i < samples_size; i++) {
				if (visited[i] == false) {
					visited[i] = true;
					std::vector<int> sol = g->searchDFS(i, c_c, ac1_ac1, ac2_ac2, c_ac2, c_ac1, ac1_ac2);
					for (auto x : sol) {
						visited[x] = true;
					}
					clusters.push_back(sol);
				}
			}
			delete[] visited;

			auto result = noe->noisy(clusters);

			if (result > max_mrr) {
#pragma omp critical
				if (result > max_mrr) {
					max_mrr = result;
					max_c_c = c_c;
					max_ac1_ac1 = ac1_ac1;
					max_ac2_ac2 = ac2_ac2;
					max_c_ac2 = c_ac2;
					max_c_ac1 = c_ac1;
					max_ac1_ac2 = ac1_ac2;
					max_cluster = clusters;
				}
			}
			delete noe;
		}
	}
}





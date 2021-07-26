#include "Clustering.h"

Clustering::Clustering(int relation, int size, Index* index, TraintripleReader* graph, TesttripleReader* ttr, ValidationtripleReader* vtr, RuleReader* rr) {
	this->strat = Properties::get().STRATEGY;
	this->portions = Properties::get().RESOLUTION;
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

	fopen_s(&chkFile, (Properties::get().PATH_CLUSTER + "_" + std::to_string(relation) + "_chk").c_str(), "w");

	

	const unsigned long long MAX_BUF = Properties::get().BUFFER_SIZE;
	long long CURR_BUF = 0;
	bool stop = false;
	int asdf = 0;

	int* vt_adj_list = vtr->getCSR()->getAdjList();
	int* vt_adj_begin = vtr->getCSR()->getAdjBegin();

	RuleGraph* rulegraph = new RuleGraph(index->getNodeSize(), graph, ttr, vtr);
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < lenRules; i++) {
		if (lenRules > 100 && (i % ((lenRules - 1) / 100)) == 0) {
			util::printProgress((double)i / (double)(lenRules - 1));
		}
		Rule& currRule = rules_adj_list[ind_ptr + i];
		if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::XRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSSingleStart_filt(false, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, results_vec, false, true);
			if(results_vec.size() >= Properties::get().DISCRIMINATION_BOUND){results_vec.clear();}
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac2() && currRule.getRuletype() == Ruletype::YRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSSingleStart_filt(true, *currRule.getHeadconstant(), *currRule.getBodyconstantId(), currRule, true, results_vec, false, true);
			if(results_vec.size() >= Properties::get().DISCRIMINATION_BOUND){results_vec.clear();}
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::XRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSMultiStart_filt(false, *currRule.getHeadconstant(), currRule, true, results_vec, false, true);
			if(results_vec.size() >= Properties::get().DISCRIMINATION_BOUND){results_vec.clear();}			
			currRule.setBuffer(results_vec);
		}
		else if (currRule.is_ac1() && currRule.getRuletype() == Ruletype::YRule) {
			std::vector<int> results_vec;
			rulegraph->searchDFSMultiStart_filt(true, *currRule.getHeadconstant(), currRule, true, results_vec, false, true);
			if(results_vec.size() >= Properties::get().DISCRIMINATION_BOUND){results_vec.clear();}		
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
									if(tailresults_vec.size() >= Properties::get().DISCRIMINATION_BOUND){tailresults_vec.clear();}									
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
									if(headresults_vec.size() >= Properties::get().DISCRIMINATION_BOUND){headresults_vec.clear();}								
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

	if (Properties::get().STRATEGY.compare("vs") == 0) {
		learn_vs(rulegraph);
	}
	else {
		std::vector<std::pair<int, double>>* jacc = read_jaccard(jacc_path);
		Graph* g = new Graph(samples_size, jacc, rules_adj_list, ind_ptr);
		std::cout << "Calculated jaccards\n";
		learn_parameters(g, rulegraph);
		delete[] jacc;
		delete g;
	}

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


	fclose(chkFile);

	std::cout << "Calculated parameters\n";

	std::cout << "MAX Head " << max_c_c_head << " " << max_ac1_ac1_head << " " << max_ac2_ac2_head << " " << max_c_ac2_head << " " << max_c_ac1_head << " " << max_ac1_ac2_head << " " << max_mrr_head << "\n";
	std::ostringstream stringStream;
	stringStream << "Relation\tHead\t" << *index->getStringOfRelId(relation) << "\t" << max_c_c_head << " " << max_ac1_ac1_head << " " << max_ac2_ac2_head << " " << max_c_ac2_head << " " << max_c_ac1_head << " " << max_ac1_ac2_head << " " << max_mrr_head << "\n";
	for (auto cluster : max_cluster_head) {
		for (auto rule : cluster) {
			Rule& r = rules_adj_list[ind_ptr + rule];
			stringStream << r.getRulestring() << "\t";
		}
		stringStream << "\n";
	}
	stringStream << "\n";

	std::cout << "MAX Tail " << max_c_c_tail << " " << max_ac1_ac1_tail << " " << max_ac2_ac2_tail << " " << max_c_ac2_tail << " " << max_c_ac1_tail << " " << max_ac1_ac2_tail << " " << max_mrr_tail << "\n";
	stringStream << "Relation\tTail\t" << *index->getStringOfRelId(relation) << "\t" << max_c_c_tail << " " << max_ac1_ac1_tail << " " << max_ac2_ac2_tail << " " << max_c_ac2_tail << " " << max_c_ac1_tail << " " << max_ac1_ac2_tail << " " << max_mrr_tail << "\n";
	for (auto cluster : max_cluster_tail) {
		for (auto rule : cluster) {
			Rule& r = rules_adj_list[ind_ptr + rule];
			stringStream << r.getRulestring() << "\t";
		}
		stringStream << "\n";
	}
	stringStream << "\n";

	

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

void Clustering::learn_vs(RuleGraph* rulegraph) {
	{
		ApplicationEngine* noe = new ApplicationEngine(relation, rulegraph, index, graph, ttr, vtr, rr);
		std::vector<std::vector<int>> clusters;
		std::vector<int> cluster;
		for (int i = 0; i < samples_size; i++) {
			Rule& r = rules_adj_list[ind_ptr + i];
			cluster.push_back(i);
		}
		clusters.push_back(cluster);

		double result_head, result_tail;
		std::tie(result_head, result_tail) = noe->max(clusters);

		if (result_head > max_mrr_head) {

#pragma omp critical
			if (result_head > max_mrr_head) {
				max_mrr_head = result_head;
				max_c_c_head = 0.0;
				max_ac1_ac1_head = 0.0;
				max_ac2_ac2_head = 0.0;
				max_c_ac2_head = 0.0;
				max_c_ac1_head = 0.0;
				max_ac1_ac2_head = 0.0;
				max_cluster_head = clusters;
				std::ostringstream os;
				os << *index->getStringOfRelId(relation) << "MAX Head " << max_c_c_head << " " << max_ac1_ac1_head << " " << max_ac2_ac2_head << " " << max_c_ac2_head << " " << max_c_ac1_head << " " << max_ac1_ac2_head << " " << max_mrr_head << "\n";
				writeThreshChk(os.str());
			}
		}
		if (result_tail > max_mrr_tail) {

#pragma omp critical
			if (result_tail > max_mrr_tail) {
				max_mrr_tail = result_tail;
				max_c_c_tail = 0.0;
				max_ac1_ac1_tail = 0.0;
				max_ac2_ac2_tail = 0.0;
				max_c_ac2_tail = 0.0;
				max_c_ac1_tail = 0.0;
				max_ac1_ac2_tail = 0.0;
				max_cluster_tail = clusters;

				std::ostringstream os;
				os << *index->getStringOfRelId(relation) << " MAX Tail " << max_c_c_tail << " " << max_ac1_ac1_tail << " " << max_ac2_ac2_tail << " " << max_c_ac2_tail << " " << max_c_ac1_tail << " " << max_ac1_ac2_tail << " " << max_mrr_tail << "\n";
				writeThreshChk(os.str());
			}
		}

		delete noe;
	}

	{
		ApplicationEngine* noe = new ApplicationEngine(relation, rulegraph, index, graph, ttr, vtr, rr);
		std::vector<std::vector<int>> clusters;
		for (int i = 0; i < samples_size; i++) {
			Rule& r = rules_adj_list[ind_ptr + i];
			std::vector<int> cluster;
			cluster.push_back(i);
			clusters.push_back(cluster);
		}

		double result_head, result_tail;
		std::tie(result_head, result_tail) = noe->noisy(clusters);

		if (result_head > max_mrr_head) {

#pragma omp critical
			if (result_head > max_mrr_head) {
				max_mrr_head = result_head;
				max_c_c_head = 1.0;
				max_ac1_ac1_head = 1.0;
				max_ac2_ac2_head = 1.0;
				max_c_ac2_head = 1.0;
				max_c_ac1_head = 1.0;
				max_ac1_ac2_head = 1.0;
				max_cluster_head = clusters;

				std::ostringstream os;
				os << *index->getStringOfRelId(relation) << "MAX Head " << max_c_c_head << " " << max_ac1_ac1_head << " " << max_ac2_ac2_head << " " << max_c_ac2_head << " " << max_c_ac1_head << " " << max_ac1_ac2_head << " " << max_mrr_head << "\n";
				writeThreshChk(os.str());
			}
		}
		if (result_tail > max_mrr_tail) {

#pragma omp critical
			if (result_tail > max_mrr_tail) {
				max_mrr_tail = result_tail;
				max_c_c_tail = 1.0;
				max_ac1_ac1_tail = 1.0;
				max_ac2_ac2_tail = 1.0;
				max_c_ac2_tail = 1.0;
				max_c_ac1_tail = 1.0;
				max_ac1_ac2_tail = 1.0;
				max_cluster_tail = clusters;
				std::ostringstream os;
				os << *index->getStringOfRelId(relation) << "MAX Tail " << max_c_c_tail << " " << max_ac1_ac1_tail << " " << max_ac2_ac2_tail << " " << max_c_ac2_tail << " " << max_c_ac1_tail << " " << max_ac1_ac2_tail << " " << max_mrr_tail << "\n";
				writeThreshChk(os.str());
			}
		}

		delete noe;
	}
}





void Clustering::learn_parameters(Graph * g, RuleGraph * rulegraph) {

	int iterations;
	if (strat.compare("grid") == 0) {
		iterations = portions + 1;
	}
	else if (strat.compare("random") == 0) {
		iterations = Properties::get().ITERATIONS;
		random_sample = new std::vector<double>[iterations];
		util::sample_random(random_sample, iterations, portions);
	}
	else {
		throw std::runtime_error("Strategy not supported");
	}


#pragma omp parallel for schedule(dynamic)
	for(int i = 0; i < iterations; i++){
		if (iterations > 100 && (i % ((iterations - 1) / 100)) == 0) {
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

			double result_head, result_tail;
			std::tie(result_head, result_tail) = noe->max(clusters);

			if (result_head > max_mrr_head) {
				
#pragma omp critical
				if (result_head > max_mrr_head) {
					max_mrr_head = result_head;
					max_c_c_head = 0.0;
					max_ac1_ac1_head = 0.0;
					max_ac2_ac2_head = 0.0;
					max_c_ac2_head = 0.0;
					max_c_ac1_head = 0.0;
					max_ac1_ac2_head = 0.0;
					max_cluster_head = clusters;
					std::ostringstream os;
					os << *index->getStringOfRelId(relation) << "MAX Head " << max_c_c_head << " " << max_ac1_ac1_head << " " << max_ac2_ac2_head << " " << max_c_ac2_head << " " << max_c_ac1_head << " " << max_ac1_ac2_head << " " << max_mrr_head << "\n";
					writeThreshChk(os.str());
				}
			}
			if (result_tail > max_mrr_tail) {
				
#pragma omp critical
				if (result_tail > max_mrr_tail) {
					max_mrr_tail = result_tail;
					max_c_c_tail = 0.0;
					max_ac1_ac1_tail = 0.0;
					max_ac2_ac2_tail = 0.0;
					max_c_ac2_tail = 0.0;
					max_c_ac1_tail = 0.0;
					max_ac1_ac2_tail = 0.0;
					max_cluster_tail = clusters;

					std::ostringstream os;
					os << *index->getStringOfRelId(relation) << " MAX Tail " << max_c_c_tail << " " << max_ac1_ac1_tail << " " << max_ac2_ac2_tail << " " << max_c_ac2_tail << " " << max_c_ac1_tail << " " << max_ac1_ac2_tail << " " << max_mrr_tail << "\n";
					writeThreshChk(os.str());
				}
			}

			delete noe;
		} else {
			double c_c, ac1_ac1, ac2_ac2, c_ac2, c_ac1, ac1_ac2;
			if(strat.compare("grid")==0){
				double thresh = (double)i / portions;
				c_c = thresh;
				ac1_ac1 = thresh;
				ac2_ac2 = thresh;
				c_ac2 = thresh;
				c_ac1 = thresh;
				ac1_ac2 = thresh;
			}
			else if (strat.compare("random")==0) {
				c_c = random_sample[i][0];
				ac1_ac1 = random_sample[i][1];
				ac2_ac2 = random_sample[i][2];
				c_ac2 = random_sample[i][3];
				c_ac1 = random_sample[i][4];
				ac1_ac2 = random_sample[i][5];
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

			double result_head, result_tail;
			std::tie(result_head, result_tail) = noe->noisy(clusters);

			if (result_head > max_mrr_head) {
				
#pragma omp critical
				if (result_head > max_mrr_head) {
					max_mrr_head = result_head;
					max_c_c_head = c_c;
					max_ac1_ac1_head = ac1_ac1;
					max_ac2_ac2_head = ac2_ac2;
					max_c_ac2_head = c_ac2;
					max_c_ac1_head = c_ac1;
					max_ac1_ac2_head = ac1_ac2;
					max_cluster_head = clusters;

					std::ostringstream os;
					os << *index->getStringOfRelId(relation) << "MAX Head " << max_c_c_head << " " << max_ac1_ac1_head << " " << max_ac2_ac2_head << " " << max_c_ac2_head << " " << max_c_ac1_head << " " << max_ac1_ac2_head << " " << max_mrr_head << "\n";
					writeThreshChk(os.str());
				}
			}
			if (result_tail > max_mrr_tail) {
				
#pragma omp critical
				if (result_tail > max_mrr_tail) {
					max_mrr_tail = result_tail;
					max_c_c_tail = c_c;
					max_ac1_ac1_tail = ac1_ac1;
					max_ac2_ac2_tail = ac2_ac2;
					max_c_ac2_tail = c_ac2;
					max_c_ac1_tail = c_ac1;
					max_ac1_ac2_tail = ac1_ac2;
					max_cluster_tail = clusters;
					std::ostringstream os;
					os << *index->getStringOfRelId(relation) << "MAX Tail " << max_c_c_tail << " " << max_ac1_ac1_tail << " " << max_ac2_ac2_tail << " " << max_c_ac2_tail << " " << max_c_ac1_tail << " " << max_ac1_ac2_tail << " " << max_mrr_tail << "\n";
					writeThreshChk(os.str());
				}
			}

			delete noe;
		}
	}
}

void Clustering::writeThreshChk(std::string result) {
	fprintf(chkFile, "%s", result.c_str());
	fflush(chkFile);
}




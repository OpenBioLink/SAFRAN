#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>  

class Properties {

public:
	// ACTION
	std::string ACTION = "applymax";

	// PATHS
	std::string PATH_TRAINING = "train.txt";
	std::string PATH_TEST = "test.txt";
	std::string PATH_VALID = "valid.txt";
	std::string PATH_RULES = "rules";
	std::string PATH_OUTPUT = "predictions";
	std::string PATH_CLUSTER = "cluster";
	std::string PATH_JACCARD = "jaccard";

	// GENERAL
	int REL_SIZE;
	int TOP_K_OUTPUT = 10;
	int WORKER_THREADS = -1;
	int TRIAL_SIZE = 100000;
	int DISCRIMINATION_BOUND = 1000;
	std::string REFLEXIV_TOKEN = "me_myself_i"; 
	int UNSEEN_NEGATIVE_EXAMPLES = 5;
	int ONLY_UNCONNECTED = 0;
	int ONLY_XY = 0;

	// JACCARD
	std::string CLUSTER_SET = "train";
	// JACCARD + LEARNNR
	int RESOLUTION = 200;
	int SEED = -1;


	// LEARNNR
	unsigned long long BUFFER_SIZE = std::numeric_limits<unsigned long long>().max();
	std::string STRATEGY = "grid";
	int ITERATIONS = 10000;

	//trial
	int TRIAL = 0;
	int CONFIDENCE_LEVEL = 95;
	int MARGIN_OF_ERROR = 5;
	std::string PATH_TEST_SAMPLE = "test_sample.txt";
	
	static Properties& get()
	{
		static Properties instance;
		return instance;
	}

	bool read(const char* propertiespath) {
		std::ifstream is(propertiespath);
		if (!is.is_open()) return false;
		while (!is.eof()) {
			std::string strLine;
			getline(is, strLine);
			int nPos = strLine.find('=');
			if (std::string::npos == nPos) continue;
			std::string strKey = trim(strLine.substr(0, nPos));
			std::string strVal = trim(strLine.substr(nPos + 1, strLine.length() - nPos + 1));

			if (strKey.compare("PATH_TRAINING") == 0) {
				PATH_TRAINING = strVal;
			}
			else if (strKey.compare("PATH_TEST") == 0) {
				PATH_TEST = strVal;
			}
			else if (strKey.compare("PATH_VALID") == 0) {
				PATH_VALID = strVal;
			}
			else if (strKey.compare("PATH_RULES") == 0) {
				PATH_RULES = strVal;
			}
			else if (strKey.compare("PATH_OUTPUT") == 0) {
				PATH_OUTPUT = strVal;
			}
			else if (strKey.compare("UNSEEN_NEGATIVE_EXAMPLES") == 0) {
				UNSEEN_NEGATIVE_EXAMPLES = std::stoi(strVal);
			}
			else if (strKey.compare("TOP_K_OUTPUT") == 0) {
				TOP_K_OUTPUT = std::stoi(strVal);
			}
			else if (strKey.compare("WORKER_THREADS") == 0) {
				WORKER_THREADS = std::stoi(strVal);
			}
			else if (strKey.compare("TRIAL_SIZE") == 0) {
				TRIAL_SIZE = std::stoi(strVal);
			}
			else if (strKey.compare("DISCRIMINATION_BOUND") == 0) {
				DISCRIMINATION_BOUND = std::stoi(strVal);
			}
			else if (strKey.compare("TRIAL") == 0) {
				TRIAL = std::stoi(strVal);
			}
			else if (strKey.compare("CONFIDENCE_LEVEL") == 0) {
				CONFIDENCE_LEVEL = std::stoi(strVal);
			}
			else if (strKey.compare("MARGIN_OF_ERROR") == 0) {
				MARGIN_OF_ERROR = std::stoi(strVal);
			}
			else if (strKey.compare("PATH_TEST_SAMPLE") == 0) {
				PATH_TEST_SAMPLE = strVal;
			}
			else if (strKey.compare("PATH_CLUSTER") == 0) {
				PATH_CLUSTER = strVal;
			}
			else if (strKey.compare("PATH_JACCARD") == 0) {
				PATH_JACCARD = strVal;
			}
			else if (strKey.compare("ONLY_UNCONNECTED") == 0) {
				ONLY_UNCONNECTED = std::stoi(strVal);
			}
			else if (strKey.compare("CLUSTER_SET") == 0) {
				CLUSTER_SET = strVal;
			}
			else if (strKey.compare("BUFFER_SIZE") == 0) {
				BUFFER_SIZE = std::stoull(strVal);
			}
			else if (strKey.compare("REFLEXIV_TOKEN") == 0) {
				REFLEXIV_TOKEN = strVal;
			}
			else if (strKey.compare("STRATEGY") == 0) {
				STRATEGY = strVal;
			}
			else if (strKey.compare("ITERATIONS") == 0) {
				ITERATIONS = std::stoi(strVal);
			}
			else if (strKey.compare("RESOLUTION") == 0) {
				RESOLUTION = std::stoi(strVal);
			}
			else if (strKey.compare("SEED") == 0) {
				SEED = std::stoi(strVal);
			}
			else if (strKey.compare("ONLY_XY") == 0) {
				ONLY_XY = std::stoi(strVal);
			}
			else {
				std::cout << "Properties key " << strKey << " not recognized";
				exit(-1);
			}
		}

		return true;
	}

	std::string toString() {
		std::ostringstream string_rep;

		string_rep << "ACTION = " << ACTION << std::endl;

		// PATHS
		string_rep << "PATH_TRAINING = " << PATH_TRAINING << std::endl;
		string_rep << "PATH_TEST = " << PATH_TEST << std::endl;
		string_rep << "PATH_VALID = " << PATH_VALID << std::endl;
		string_rep << "PATH_RULES = " << PATH_RULES << std::endl;
		if (ACTION.compare("learnnrnoisy") == 0 || ACTION.compare("calcjacc") == 0) {
			string_rep << "PATH_JACCARD = " << PATH_JACCARD << std::endl;
		}
		if (ACTION.compare("learnnrnoisy") == 0 || ACTION.compare("applynrnoisy") == 0) {
			string_rep << "PATH_CLUSTER = " << PATH_CLUSTER << std::endl;
		}
		if (ACTION.compare("applymax") == 0 || ACTION.compare("applynoisy") == 0 || ACTION.compare("applynrnoisy") == 0) {
			string_rep << "PATH_OUTPUT = " << PATH_OUTPUT << std::endl;
		}

		// GENERAL PROPS
		string_rep << "WORKER_THREADS = " << WORKER_THREADS << std::endl;
		string_rep << "DISCRIMINATION_BOUND = " << DISCRIMINATION_BOUND << std::endl;
		string_rep << "UNSEEN_NEGATIVE_EXAMPLES = " << UNSEEN_NEGATIVE_EXAMPLES << std::endl;
		string_rep << "TOP_K_OUTPUT = " << TOP_K_OUTPUT << std::endl;
		string_rep << "REFLEXIV_TOKEN = " << REFLEXIV_TOKEN << std::endl;

		// SPECIFIC PROPS
		if (ACTION.compare("applymax") == 0 || ACTION.compare("applynoisy") == 0 || ACTION.compare("applynrnoisy") == 0 || ACTION.compare("learnnrnoisy") == 0) {
			string_rep << "ONLY_UNCONNECTED = " << ONLY_UNCONNECTED << std::endl;
		}

		if (ACTION.compare("learnnrnoisy") == 0) {
			string_rep << "STRATEGY = " << STRATEGY << std::endl;
			if (STRATEGY.compare("random") == 0) {
				string_rep << "ITERATIONS = " << ITERATIONS << std::endl;
			}
		}
		if (ACTION.compare("learnnrnoisy") == 0 || ACTION.compare("calcjacc") == 0) {
			string_rep << "RESOLUTION = " << RESOLUTION << std::endl;
			string_rep << "SEED = " << SEED << std::endl;
		}
		
		if (ACTION.compare("learnnrnoisy") == 0) {
			string_rep << "BUFFER_SIZE = " << BUFFER_SIZE << std::endl;
		}
		
		if (ACTION.compare("calcjacc") == 0) {
			string_rep << "CLUSTER_SET = " << CLUSTER_SET << std::endl;
		}

		if (ACTION.compare("applymax") == 0 || ACTION.compare("applynoisy") == 0 || ACTION.compare("applynrnoisy") == 0) {
			string_rep << "TRIAL = " << TRIAL << std::endl;
			if (TRIAL == 1) {
				string_rep << "TRIAL_SIZE = " << TRIAL_SIZE << std::endl;
				string_rep << "CONFIDENCE_LEVEL = " << CONFIDENCE_LEVEL << std::endl;
				string_rep << "MARGIN_OF_ERROR = " << MARGIN_OF_ERROR << std::endl;
				string_rep << "PATH_TEST_SAMPLE = " << PATH_TEST_SAMPLE << std::endl;
			}
		}
		
		

		return string_rep.str().c_str();
	}

private:
	Properties() {};
	Properties(const Properties&);

	const std::string WHITESPACE = " \n\r\t\f\v";

	std::string ltrim(const std::string& s)
	{
		size_t start = s.find_first_not_of(WHITESPACE);
		return (start == std::string::npos) ? "" : s.substr(start);
	}

	std::string rtrim(const std::string& s)
	{
		size_t end = s.find_last_not_of(WHITESPACE);
		return (end == std::string::npos) ? "" : s.substr(0, end + 1);
	}

	std::string trim(const std::string& s)
	{
		return rtrim(ltrim(s));
	}
};

#endif //PROPERTIES_H


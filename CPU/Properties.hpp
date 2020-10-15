#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>  

class Properties{

public:
	std::string ACTION = "applymax";

	std::string PATH_TRAINING = "train.txt";
	std::string PATH_TEST = "test.txt";
	std::string PATH_VALID = "valid.txt";
	std::string PATH_RULES = "rules";
	std::string PATH_OUTPUT = "predictions";
	std::string PATH_CLUSTER = "cluster";

	// rule application
	int TRIAL_SIZE = 100000;
	int DISCRIMINATION_BOUND = 1000;

	//noisy
	std::string CLUSTER_SET = "train";
	int REL_SIZE = 0;
	int UNSEEN_NEGATIVE_EXAMPLES = 5;
	int ONLY_UNCONNECTED = 0;
	int TOP_K_OUTPUT = 10;
	int WORKER_THREADS = 3;
	int THRESHOLD_CORRECT_PREDICTIONS = 2;

	double THRESHOLD_CONFIDENCE = 0.0001;

	int CLUSTER_LIMIT = 0;

	//performance
	int DISCRIMINATION_UNIQUE = 1;
	int FAST = 0;
	int INTERMEDIATE_DISCRIMINATION = 0;

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
			} else if (strKey.compare("PATH_TEST") == 0) {
				PATH_TEST = strVal;
			}
			else if(strKey.compare("PATH_VALID") == 0) {
				PATH_VALID = strVal;
			}
			else if (strKey.compare("PATH_RULES") == 0) {
				PATH_RULES = strVal;
			}
			else if(strKey.compare("PATH_OUTPUT") == 0) {
				PATH_OUTPUT = strVal;
			}
			else if(strKey.compare("UNSEEN_NEGATIVE_EXAMPLES") == 0) {
				UNSEEN_NEGATIVE_EXAMPLES = std::stoi(strVal);
			}
			else if(strKey.compare("TOP_K_OUTPUT") == 0) {
				TOP_K_OUTPUT = std::stoi(strVal);
			}
			else if(strKey.compare("WORKER_THREADS") == 0) {
				WORKER_THREADS = std::stoi(strVal);
			}
			else if(strKey.compare("THRESHOLD_CORRECT_PREDICTIONS") == 0) {
				THRESHOLD_CORRECT_PREDICTIONS = std::stoi(strVal);
			}
			else if(strKey.compare("THRESHOLD_CONFIDENCE") == 0) {
				THRESHOLD_CONFIDENCE = std::stod(strVal);
			}
			else if (strKey.compare("TRIAL_SIZE") == 0) {
				TRIAL_SIZE = std::stoi(strVal);
			}
			else if (strKey.compare("DISCRIMINATION_BOUND") == 0) {
				DISCRIMINATION_BOUND = std::stoi(strVal);
			}
			else if (strKey.compare("DISCRIMINATION_UNIQUE") == 0) {
				DISCRIMINATION_UNIQUE = std::stoi(strVal);
			}
			else if (strKey.compare("FAST") == 0) {
				FAST = std::stoi(strVal);
			}
			else if (strKey.compare("INTERMEDIATE_DISCRIMINATION") == 0) {
				INTERMEDIATE_DISCRIMINATION = std::stoi(strVal);
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
			else if (strKey.compare("ONLY_UNCONNECTED") == 0) {
				ONLY_UNCONNECTED = std::stoi(strVal);
			}
			else if (strKey.compare("CLUSTER_LIMIT") == 0) {
				CLUSTER_LIMIT = std::stoi(strVal);
			}
			else if (strKey.compare("CLUSTER_SET") == 0) {
				CLUSTER_SET = strVal;
			}
			else {
				std::cout << "Properties key "  << strKey <<" not recognized";
				exit(-1);
			}
		}

		return true;
	}

	std::string toString() {
		std::ostringstream string_rep;

		string_rep << "ACTION = " << ACTION << std::endl;
		string_rep << "PATH_TRAINING = " << PATH_TRAINING << std::endl;
		string_rep << "PATH_TEST = " << PATH_TEST << std::endl;
		string_rep << "PATH_VALID = " << PATH_VALID << std::endl;
		string_rep << "PATH_RULES = " << PATH_RULES << std::endl;
		string_rep << "PATH_OUTPUT = " << PATH_OUTPUT << std::endl;
		string_rep << "PATH_CLUSTER = " << PATH_CLUSTER << std::endl;
		string_rep << "TRIAL_SIZE = " << TRIAL_SIZE << std::endl;
		string_rep << "DISCRIMINATION_BOUND = " << DISCRIMINATION_BOUND << std::endl;
		string_rep << "UNSEEN_NEGATIVE_EXAMPLES = " << UNSEEN_NEGATIVE_EXAMPLES << std::endl;
		string_rep << "TOP_K_OUTPUT = " << TOP_K_OUTPUT << std::endl;
		string_rep << "WORKER_THREADS = " << WORKER_THREADS << std::endl;
		string_rep << "THRESHOLD_CORRECT_PREDICTIONS = " << THRESHOLD_CORRECT_PREDICTIONS << std::endl;
		string_rep << "THRESHOLD_CONFIDENCE = " << THRESHOLD_CONFIDENCE << std::endl;
		string_rep << "ONLY_UNCONNECTED = " << ONLY_UNCONNECTED << std::endl;
		string_rep << "DISCRIMINATION_UNIQUE = " << DISCRIMINATION_UNIQUE << std::endl;
		string_rep << "FAST = " << FAST << std::endl;
		string_rep << "INTERMEDIATE_DISCRIMINATION = " << INTERMEDIATE_DISCRIMINATION << std::endl;
		string_rep << "TRIAL = " << TRIAL << std::endl;
		string_rep << "CONFIDENCE_LEVEL = " << CONFIDENCE_LEVEL << std::endl;
		string_rep << "MARGIN_OF_ERROR = " << MARGIN_OF_ERROR << std::endl;
		string_rep << "PATH_TEST_SAMPLE = " << PATH_TEST_SAMPLE << std::endl;
		string_rep << "CLUSTER_LIMIT = " << CLUSTER_LIMIT << std::endl;


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


#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <string>
#include <algorithm>

class Properties{

public:
	std::string PATH_TRAINING;
	std::string PATH_TEST;
	std::string PATH_VALID;
	std::string PATH_RULES;
	std::string PATH_OUTPUT;

	int TRIAL_SIZE = 100000;
	int DISCRIMINATION_BOUND = 1000;
	int UNSEEN_NEGATIVE_EXAMPLES = 5;
	int TOP_K_OUTPUT = 10;
	int WORKER_THREADS = 3;
	int THRESHOLD_CORRECT_PREDICTIONS = 2;

	double THRESHOLD_CONFIDENCE = 0.0001;

	//performance

	int DISCRIMINATION_UNIQUE = 1;
	int FAST = 0;
	int INTERMEDIATE_DISCRIMINATION = 0;

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
			else {
				std::cout << "Properties key "  << strKey <<" not recognized";
				exit(-1);
			}
		}

		return true;
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


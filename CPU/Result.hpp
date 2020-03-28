#ifndef RESULT_H
#define RESULT_H

#include <vector>

class Result
{

public:
	void setTailResults(std::vector<int>& tailResults) {
		this->tailResults = tailResults;
		isTailSet = true;
	}

	void setHeadResults(std::vector<int>& headResults) {
		this->headResults = headResults;
		isHeadSet = true;
	}

	bool isComplete() {
		return isTailSet && isHeadSet;
	}

private:
	std::vector<int> tailResults;
	bool isTailSet = false;
	std::vector<int> headResults;
	bool isHeadSet = false;
};

#endif RESULT_H


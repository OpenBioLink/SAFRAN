#ifndef TRIAL_H
#define TRIAL_H

#include <iostream>
#include <cmath>
#include <random>
#include <string>
#include <iterator>
#include <algorithm>

class Trial {

public:
	Trial(int confidence_level, int margin_of_error, int population);
	std::vector<std::vector<int*>> getTesttriplesSample(std::vector<std::vector<int*>> allTesttriples);

private:
	int sample_size;

	int getSampleSize(int confidence_level, int margin_of_error, int population);
	double getZ(int confidence_level);
};

#endif //TRIAL_H
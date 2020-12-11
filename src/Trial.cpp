#include "Trial.h"

Trial::Trial(int confidence_level, int margin_of_error, int population) {
	sample_size = getSampleSize(confidence_level, margin_of_error, population);
	std::cout << "Sample size is " << sample_size << std::endl;
}

std::vector<std::vector<int*>> Trial::getTesttriplesSample(std::vector<std::vector<int*>> allTesttriples) {
	std::cout << "Sampling " << sample_size << " random testtriples" << std::endl;
	std::vector<std::vector<int*>> out;
	//std::sample(allTesttriples.begin(), allTesttriples.end(), std::back_inserter(out),
	//	sample_size, std::mt19937{ std::random_device{}() });
	return out;
}

int Trial::getSampleSize(int confidence_level, int margin_of_error, int population) {
	double z = getZ(confidence_level);
	double confidence_level_dec = (double)confidence_level / 100.0;
	double margin_of_error_dec = (double)margin_of_error / 100.0;

	std::cout << "Confidence level: " << confidence_level_dec << " Margin of error: " << margin_of_error_dec << " z-Score: " << z << std::endl;

	double numerator = (std::pow(z, 2.0) * 0.25) / std::pow(margin_of_error_dec, 2.0);
	double denominator = 1.0 + ((std::pow(z, 2.0) * 0.25) / (std::pow(margin_of_error_dec, 2.0) * (double)population));
	return ceil(numerator / denominator);
}

double Trial::getZ(int confidence_level) {
	if (confidence_level == 99) {
		return 2.58;
	} 
	else if (confidence_level == 95) {
		return 1.96;
	}
	else if (confidence_level == 90) {
		return 1.65;
	}
	else if (confidence_level == 85) {
		return 1.44;
	}
	else if (confidence_level == 80) {
		return 1.28;
	}
	else {
		std::cout << "Confidence level " << confidence_level << "% not supported.";
		exit(-1);
	}
}
#include "MinHash.h"

MinHash::MinHash(int k) {
	this->k = k;
	std::mt19937 generator(Properties::get().SEED);
	std::uniform_int_distribution<long long> distribution_0(0, _mersenne_prime - 1);
	std::uniform_int_distribution<long long> distribution_1(1, _mersenne_prime - 1);
	permfuncs = new long long* [k];
	for (int i = 0; i < k; i++) {
		permfuncs[i] = new long long[2];

		long long next_a = distribution_1(generator);

		long long next_b = distribution_0(generator);

		permfuncs[i][0] = next_a;
		permfuncs[i][1] = next_b;
	}
}

MinHash::~MinHash() {
	for (int i = 0; i < k; i++) {
		delete[] permfuncs[i];
	}
	delete[] permfuncs;
}

std::vector<long long> MinHash::getMinimum(std::vector<std::vector<int>>& heads, std::vector<std::vector<int>>& tails){
	std::vector<long long> solutions(k);
	for (int i = 0; i < k; i++) {
		solutions[i] = std::numeric_limits<long long>::max();
	}


	for (int j = 0; j < heads.size(); j++) {
		for (auto head : heads[j]) {
			for (auto tail : tails[j]) {
				unsigned long long a = head;
				unsigned long long b = tail;
				unsigned long long cantor_pairing = (a + b) * (a + b + 1) / 2 + a;
				unsigned long long h_v = hash(cantor_pairing);
				for (int i = 0; i < k; i++) {
					unsigned long long hash = fast_mod((permfuncs[i][0] * h_v) + permfuncs[i][1], _mersenne_prime) & _max_hash;
					if (hash < solutions[i]) {
						solutions[i] = hash;
					}
				}
			}
		}
	}
	return solutions;
}

unsigned long long MinHash::fast_mod(const long long input, const long long ceil) {
	unsigned long long val = input;
	val = (val & _mersenne_prime) + (val >> 61);
	val = (val & _mersenne_prime) + (val >> 61); //necessary if previous add led to a val > M61
	val = (val == _mersenne_prime) ? 0 : val;
	return val;
}



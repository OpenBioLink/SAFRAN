#ifndef MINHASH
#define MINHASH

#include <iostream>
#include <string>

#include <algorithm> 
#include <chrono>
#include <random>
#include <limits>
#include <time.h>
#include "Properties.hpp"

class MinHash {
public:
	MinHash(int k);
	~MinHash();

	std::vector<long long> getMinimum(std::vector<std::vector<int>>& heads, std::vector<std::vector<int>>& tails);

private:
	long long _mersenne_prime = ((long long)1 << 61) - 1;
	long long _max_hash = ((long long)1 << 32) - 1;
	unsigned long long fast_mod(const long long input, const long long ceil);
	std::hash<long long> hash;
	long long** permfuncs;
	int k;
};


#endif // !MINHASH

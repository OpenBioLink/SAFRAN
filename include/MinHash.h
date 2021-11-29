
// Implementation based on:
// CS494 Lecture Notes - MinHash
// Author: James S. Plank
// https://web.eecs.utk.edu/~jplank/plank/classes/cs494/494/notes/Min-Hash/index.html

#ifndef MIN_HASH
#define MIN_HASH

#include <string>
#include <cstring>
#include <stdint.h> 
#include <fstream>
#include <vector>
#include <list>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <openssl/md5.h>

class MinHash
{
private:
    int k;
public:
    MinHash(int k);
    ~MinHash();
	std::set<unsigned long long> getMinimum(std::vector<std::vector<int>>& heads, std::vector<std::vector<int>>& tails);
	double getJacc(std::set<unsigned long long>& a, std::set<unsigned long long>& b);
};

#endif


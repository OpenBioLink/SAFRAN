#include "MinHash.h"

MinHash::MinHash(int k)
{
    this->k = k;
}

std::set<unsigned long long> MinHash::getMinimum(std::vector<std::vector<int>>& heads, std::vector<std::vector<int>>& tails){
    std::set<unsigned long long> min_hash;  // The k minimum hashes for each file.
    unsigned char hash[16];  
    unsigned long long ll;
    std::set<unsigned long long>::iterator liti, litj;      
    for (int j = 0; j < heads.size(); j++) {
        for (auto head : heads[j]) {
            for (auto tail : tails[j]) {
                std::string s = std::to_string(head) + "+" + std::to_string(tail) + ";";
                MD5((unsigned char *) s.c_str(), s.size(), hash);
                memcpy((unsigned char *) &ll, hash, sizeof(long long));
                
                /* Error check code 1: Print out the hashes. */
                // printf("%-20s 0x%016llx\n", s.c_str(), ll);

                if (min_hash.size() < this->k) {
                    min_hash.insert(ll);
                } else {
                    liti = min_hash.begin();
                    if (ll > *liti) {
                        min_hash.insert(ll);
                        if (min_hash.size() > this->k) min_hash.erase(liti);
                    }
                }
            }
        }
    }
    return min_hash;
}


double MinHash::getJacc(std::set<unsigned long long>& a, std::set<unsigned long long>& b){
    std::set<unsigned long long>::iterator liti, litj;
    double Intersection;                  
    double Union;                  
    double Total;   
    liti = a.begin();
    litj = a.begin();
    Intersection = 0;
    while (liti != b.end() && litj != b.end()) {
    if (*liti == *litj) {
        Intersection++;
        liti++;
        litj++;
    } else if (*liti < *litj) {
        liti++;
    } else {
        litj++;
    }
    }
    Total = a.size() + b.size();
    Union = Total - Intersection;
    return Intersection / Union;
}

MinHash::~MinHash()
{
}


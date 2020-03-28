#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace util{
	__host__ std::vector<std::string> split(const std::string& str, char delim = ' ') {
		std::stringstream ss(str);
		std::string token;
		std::vector<std::string> cont;

		while (std::getline(ss, token, delim)) {
			cont.push_back(token);
		}
		return cont;
	}
}

#endif // UTIL_H

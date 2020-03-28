#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <sstream>

namespace util{
	inline std::vector<std::string> split(const std::string& str, char delim = ' ') {
		std::stringstream ss(str);
		std::string token;
		std::vector<std::string> cont;

		while (std::getline(ss, token, delim)) {
			cont.push_back(token);
		}
		return cont;
	}

	template<typename ... Args>
	inline std::string string_format(const std::string& format, Args ... args)
	{
		size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size <= 0) { throw std::runtime_error("Error during formatting."); }
		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}
}

#endif // UTIL_H

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

	template<class InputIt1, class InputIt2, class OutputIt>
	inline OutputIt test_set_difference(InputIt1 first1, InputIt1 last1,
		InputIt2 first2, InputIt2 last2,
		OutputIt d_first, int other)
	{
		while (first1 != last1) {
			if (first2 == last2) return std::copy(first1, last1, d_first);

			if ((*first1 < *first2) || (*first1 == other)) {
				*d_first++ = *first1++;
			}
			else {
				if (!(*first2 < *first1)) {
					++first1;
				}
				++first2;
			}
		}
		return d_first;
	}
}

#endif // UTIL_H

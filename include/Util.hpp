#ifndef UTIL_H
#define UTIL_H

#ifdef __unix
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <random>
#include <iostream>
#include "Properties.hpp"

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

	inline bool in_sorted(std::vector<int>& vector, int ele) {
		if (vector.size() == 0) return false;
		int a = 0;
		int b = (vector.size() - 1) / 2;
		int c = vector.size() - 1;


		if (vector[a] > ele) return false;
		if (vector[c] == ele) return true;
		if (vector[c] < ele) return false;
		int asdf = 0;

		while (true) {
			asdf++;
			if (asdf == 100) {
				std::cout << "INF " << ele << "\n";
				for (auto i : vector) {
					std::cout << i << " ";
				}
				exit(-1);
			}
			if (vector[b] == ele) return true;
			if (a == b or b == c) {
				break;
			}
			if (vector[a] <= ele and ele < vector[b]) {
				c = b;
				b = (a + b) / 2;
			}
			else if (vector[b] < ele and ele <= vector[c]) {
				a = b;
				b = (b + c) / 2;
			}
		}
		return false;
	}

	inline std::mt19937 get_prng() {
		std::random_device r;
		std::seed_seq seed{ r(), r(), r(), r(), r(), r(), r(), r() };
		return std::mt19937(seed);
	}

	inline void sample_random(std::vector<double> * random_sample, int iterations, int portions) {
		std::mt19937 gen(Properties::get().SEED);
		for (int i = 0; i < iterations; i++) {
			for (int j = 0; j < 6; j++) {
				double thresh = (double)(gen() % (portions + 1)) / portions;
				random_sample[i].push_back(thresh);
			}
		}
	}

	inline void printProgress(double percentage) {
		if (percentage > 1.0) percentage = 1.0;
		int val = (int)(percentage * 100);
		int lpad = (int)(percentage * PBWIDTH);
		int rpad = PBWIDTH - lpad;
		printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
		fflush(stdout);
		if (percentage >= 1.0) printf("\n");
	}

	inline std::istream& safeGetline(std::istream& is, std::string& t)
	{
		t.clear();

		// The characters in the stream are read one-by-one using a std::streambuf.
		// That is faster than reading them one-by-one using the std::istream.
		// Code that uses streambuf this way must be guarded by a sentry object.
		// The sentry object performs various tasks,
		// such as thread synchronization and updating the stream state.

		std::istream::sentry se(is, true);
		std::streambuf* sb = is.rdbuf();

		for (;;) {
			int c = sb->sbumpc();
			switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case std::streambuf::traits_type::eof():
				// Also handle the case when the last line has no line ending
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += (char)c;
			}
		}
	}

	inline int compFraction(long long num_first, long long denum_fist, long long num_second, long long denum_second)
	{
		// Compute ad-bc
		long long Y = num_first * denum_second - denum_fist * num_second;

		if (Y > 0) {
			// first is bigger
			return 1;
		}
		else if (Y < 0) {
			// second is bigger
			return -1;
		}
		else {
			// eq
			return 0;
		}
	}
}

#endif // UTIL_H

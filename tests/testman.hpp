#ifndef TESTMAN_HPP
#define TESTMAN_HPP

#include "iostream"
#include "binary-tests.hpp"
#include "network-tests.hpp"

namespace testman
{
	const char *bold = "\033[1m";
	const char *reset = "\033[0m";

	const int total_tests = 2;
	int tests_passed = 0;
	int tests_done = 0;

	inline void all_tests()
	{
		tests_done++;
		std::cout << bold << "testman: (" << tests_done << "/" << total_tests << ") running binary tests" << reset << std::endl;
		if (binary::binary())
		{
			tests_passed++;
		}

		tests_done++;
		std::cout << bold << "testman: (" << tests_done << "/" << total_tests << ") running binary tests" << reset << std::endl;
		if (network::network())
		{
			tests_passed++;
		}

		if (tests_passed == total_tests)
		{
			std::cout << std::endl
				  << bold << "testman:" << reset << " all tests passed!" << std::endl
				  << std::endl;
		}
		else
		{
			std::cout << std::endl
				  << bold << "testman:" << reset << " some tests finished with errors." << std::endl
				  << std::endl;
		}
	}
}

#endif
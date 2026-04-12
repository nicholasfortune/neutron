#ifndef TESTMAN_HPP
#define TESTMAN_HPP

#include "iostream"
#include "binary.hpp"
#include "network.hpp"

namespace testman
{
    const int total_tests = 2;
    int tests_passed = 0;
    int tests_done = 0;

    inline void all_tests()
    {
        tests_done++;
        std::cout << "\033[1mtestman: (" << tests_done << "/" << total_tests << ") running binary tests\033[0m" << std::endl;
        if (binary::binary())
        {
            tests_passed++;
        }

        tests_done++;
        std::cout << "\033[1mtestman: (" << tests_done << "/" << total_tests << ") running network tests\033[0m" << std::endl;
        if (network::network())
        {
            tests_passed++;
        }

        if (tests_passed == total_tests)
        {
            std::cout << std::endl
                      << "\033[1mtestman:\033[0m all tests passed!" << std::endl
                      << std::endl;
        }
        else
        {
            std::cout << std::endl
                      << "\033[1mtestman:\033[0m some tests finished with errors." << std::endl
                      << std::endl;
        }
    }
}

#endif
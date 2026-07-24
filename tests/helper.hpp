#ifndef HELPER_HPP
#define HELPER_HPP

#include <iostream>
#include <cstdint>
#include <vector>
#include "../neutron.hpp"

namespace helper
{
	neutron::helper::matrix test_matrix_a = neutron::helper::matrix(3, 2);
	neutron::helper::matrix test_matrix_b = neutron::helper::matrix(3, 2);

	bool get()
	{
		uint32_t fails = 0;

		// Initialize matrices with the correct test values
		test_matrix_a.data = {1.618f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
		test_matrix_b.data = {6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.414f};

		/*
		Tests:
		a: (0, 0)
		b: (2, 1)

		Expected reads:
		a: (0, 0) = 1.618f
		b: (2, 1) = 1.414f

		A pass is within 1e-3f of these expected results.
		*/

		if (std::abs(1.618f - test_matrix_a.get(0, 0)) > 1e-3f)
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: get: matrix coordinate (0, 0) not within one-thousandth of expected value.\n";
			fails++;
		}
		if (std::abs(1.414f - test_matrix_b.get(2, 1)) > 1e-3f)
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: get: matrix coordinate (2, 1) not within one-thousandth of expected value.\n";
			fails++;
		}
		return (fails == 0);
	}
	bool set()
	{
		uint32_t fails = 0;

		/*
		Set:
		a: (0, 0) = 1.0f
		b: (2, 1) = 1.0f

		Expected reads:
		a: (0, 0) = 1.0f
		b: (2, 1) = 1.0f

		A pass is within 1e-3f of these expected results.
		*/

		test_matrix_a.set(0, 0, 1.0f);
		test_matrix_b.set(2, 1, 1.0f);

		if (std::abs(1.0f - test_matrix_a.get(0, 0)) > 1e-3f)
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: get: matrix coordinate (0, 0) not within one-thousandth of expected value.\n";
			fails++;
		}
		if (std::abs(1.0f - test_matrix_b.get(2, 1)) > 1e-3f)
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: get: matrix coordinate (2, 1) not within one-thousandth of expected value.\n";
			fails++;
		}

		return (fails == 0);
	}
	bool transpose()
	{
		uint32_t fails = 0;

		/*
		Transposing matrix A.

		Expected results:
		[1, 3, 5,
		 2, 4, 6]

		A pass is within 1e-3f of these expected results.
		*/

		neutron::helper::matrix expected_results = neutron::helper::matrix(2, 3);
		expected_results.data = {1.0f, 3.0f, 5.0f, 2.0f, 4.0f, 6.0f};

		// Collect results
		neutron::helper::matrix results = test_matrix_a.transpose();

		if (results.data.size() != expected_results.data.size())
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: transpose: results and expected results are not the same size.\n";
			fails++;
		}

		if (results.rows != expected_results.rows || results.columns != expected_results.columns)
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: transpose: dimensions are not the same size.\n";
			fails++;
		}

		for (uint32_t i = 0; i < results.data.size(); i++)
		{
			if (results.data[i] != expected_results.data[i])
			{
				std::cerr << "\033[31m[ ERROR ]\033[0m helper: transpose: matrix data index " << i << " not within one-thousandth of expected value.\n";
				fails++;
			}
		}

		return (fails == 0);
	}
	bool dot()
	{
		uint32_t fails = 0;

		/*
		Transpose matrix A, then dot product the result and matrix B.

		Expected results:
		[28, 19,
		 40, 28]

		A pass is within 1e-3f of these expected results.
		*/

		neutron::helper::matrix expected_results = neutron::helper::matrix(2, 2);
		expected_results.data = {28.0f, 19.0f, 40.0f, 28.0f};

		// Collect results
		neutron::helper::matrix results = test_matrix_a.transpose().dot(test_matrix_b);

		if (results.data.size() != expected_results.data.size())
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: dot: results and expected results are not the same size.\n";
			fails++;
		}

		if (results.rows != expected_results.rows || results.columns != expected_results.columns)
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: dot: dimensions are not the same size.\n";
			fails++;
		}

		for (uint32_t i = 0; i < results.data.size(); i++)
		{
			if (results.data[i] != expected_results.data[i])
			{
				std::cerr << "\033[31m[ ERROR ]\033[0m helper: dot: matrix data index " << i << " not within one-thousandth of expected value.\n";
				fails++;
			}
		}

		return (fails == 0);
	}
	bool element()
	{
		uint32_t fails = 0;

		/*
		Do element-wise multiplication of matrix A and B.

		Expected results:
		[6, 10,
		 12, 12,
		 10, 6]

		A pass is within 1e-3f of these expected results.
		*/

		neutron::helper::matrix expected_results = neutron::helper::matrix(3, 2);
		expected_results.data = {6.0f, 10.0f, 12.0f, 12.0f, 10.0f, 6.0f};

		// Collect results
		neutron::helper::matrix results = test_matrix_a.element(test_matrix_b);

		if (results.data.size() != expected_results.data.size())
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: element: results and expected results are not the same size.\n";
			fails++;
		}

		if (results.rows != expected_results.rows || results.columns != expected_results.columns)
		{
			std::cerr << "\033[31m[ ERROR ]\033[0m helper: element: dimensions are not the same size.\n";
			fails++;
		}

		for (uint32_t i = 0; i < results.data.size(); i++)
		{
			if (results.data[i] != expected_results.data[i])
			{
				std::cerr << "\033[31m[ ERROR ]\033[0m helper: element: matrix data index " << i << " not within one-thousandth of expected value.\n";
				fails++;
			}
		}

		return (fails == 0);
	}

	bool helper()
	{
		bool success = true;
		if (!get())
		{
			std::cout << "\033[31m[ FAILED ]\033[0m helper: get()\n";
			std::cout << "\033[31m[ FATAL ]\033[0m helper: get() was required for further tests, quitting helper test.\n";
			success = false;
		}
		else
		{
			std::cout << "\033[32m[ PASSED ]\033[0m helper: get()\n";
			if (!set())
			{
				std::cout << "\033[31m[ FAILED ]\033[0m helper: set()\n";
				std::cout << "\033[31m[ FATAL ]\033[0m helper: set() was required for further tests, quitting helper test.\n";
				success = false;
			}
			else
			{
				std::cout << "\033[32m[ PASSED ]\033[0m helper: set()\n";
				if (!transpose())
				{
					std::cout << "\033[31m[ FAILED ]\033[0m helper: transpose()\n";
					success = false;
				}
				else
				{
					std::cout << "\033[32m[ PASSED ]\033[0m helper: transpose()\n";
				}
				if (!dot())
				{
					std::cout << "\033[31m[ FAILED ]\033[0m helper: dot()\n";
					success = false;
				}
				else
				{
					std::cout << "\033[32m[ PASSED ]\033[0m helper: dot()\n";
				}
				if (!element())
				{
					std::cout << "\033[31m[ FAILED ]\033[0m helper: element()\n";
					success = false;
				}
				else
				{
					std::cout << "\033[32m[ PASSED ]\033[0m helper: element()\n";
				}
			}
		}

		if (!success)
		{
			std::cout << "\033[33m[ NOTICE ]\033[0m helper: \033[1msome tests failed.\033[0m" << std::endl;
		}

		return success;
	}
}

#endif
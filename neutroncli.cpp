#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include "neutron.hpp"
#include "tests/testman.hpp"

// Constants
const char *version = "1.0.0.0";	 // 1.major.minor.bugfix
const char *copyright_date = "2026";	 // Year
const char *release_date = "04/10/2026"; // Day/Month/Year
bool bench = false;
bool shush = false;
bool force = false;

// Helpers
void print(const char *str)
{
	if (!shush)
	{
		std::cout << str;
	}
}
void println(const char *str)
{
	if (!shush)
	{
		std::cout << str << std::endl;
	}
}
bool convert_to_uint32_t(const char *str, uint32_t &out)
{
	try
	{
		unsigned long val = std::stoul(str);
		if (val > UINT32_MAX)
			return false;
		out = static_cast<uint32_t>(val);
		return true;
	}
	catch (...)
	{
		return false;
	}
}
void remove_whitespace(char *str)
{
	char *dst = str;
	while (*str)
	{
		if (!isspace(static_cast<unsigned char>(*str)))
		{
			*dst++ = *str;
		}
		str++;
	}
	*dst = '\0';
}

// Functions
void help()
{
	println("\033[1mGeneral Purpose Flags\033[0m");
	println("    --bench -b");
	println("        Displays the time taken to complete a command/task");
	println("");
	println("    --shush -s");
	println("        Stops any non-essential prints the command may make");
	println("");
	println("    --force -f");
	println("        Forces the command to continue even if there is an error/warning");
	println("");
	println("");
	println("\033[1mHelper Commands\033[0m");
	println("    help");
	println("        Displays this list of commands");
	println("");
	println("    test");
	println("        Runs all available tests");
	println("");
	println("");
	println("\033[1mMain Commands\033[0m");
	println("    create \"file-name\" <number of neurons per layer>");
	println("        Creates/overwrites an empty neural network file with the given name in the current directory");
	println("       \033[1m ex:\033[0m neutron create \"rock-paper-scissors-master.bin\" 3 4 1");
	println("");
	println("    forward \"file-name\" \"<inputs>\"");
	println("        Computes and returns the forward propagation outputs of a given neural network file using the given inputs");
	println("        \033[1mex:\033[0m neutron forward \"rock-paper-scissors-master.bin\" 3.4 0.98 3");
	println("");
	println("    read \"file-name\"");
	println("        Returns important data about a given neural network file.");
	println("");
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "Neutron " << version << " " << release_date << std::endl;
		std::cout << "Copyright " << copyright_date << " Nicholas Fortune. Licensed under GNU GPL v3." << std::endl;
		std::cout << "You are free to use, modify, and share this software under the GPL license." << std::endl;
		std::cout << "Use \"neutron help\" for help." << std::endl
			  << std::endl;
		return 0;
	}
	std::vector<char *> arguments;
	bench = false;
	shush = false;
	force = false;
	for (size_t i = 1; i < argc; i++)
	{
		if (argv[i][0] != '-')
		{
			arguments.push_back(argv[i]);
		}
		else
		{
			if (std::string(argv[i]) == "-bench")
			{
				bench = true;
			}
			else if (std::string(argv[i]) == "-shush")
			{
				shush = true;
			}
			else if (std::string(argv[i]) == "-force")
			{
				force = true;
			}
			else
			{
				print("Flag \"");
				print((char *)argv[i]);
				println("\" is not a valid flag. Run \"neutron help\" for help.");
			}
		}
	}
	std::string cmd = std::string(arguments[0]);
	arguments.erase(arguments.begin());

	auto start = std::chrono::high_resolution_clock::now();

	if (cmd == "help")
	{
		help();
	}
	else if (cmd == "version")
	{
		print("neutron version ");
		println(version);
	}
	else if (cmd == "create")
	{
		if (arguments.size() <= 2 && !force)
		{
			println("error: too few arguments");
		}
		else
		{
			std::vector<uint32_t> layer_sizes;
			layer_sizes.resize(arguments.size() - 1);

			for (size_t i = 0; i < layer_sizes.size(); i++)
			{
				unsigned long ul = std::stoul(arguments[i + 1]);
				layer_sizes[i] = static_cast<uint32_t>(std::min<unsigned long>(ul, std::numeric_limits<uint32_t>::max()));
			}
			neutron::network new_network = neutron::network(layer_sizes);
			new_network.save_network(arguments[0]);
		}
	}
	else if (cmd == "test")
	{
		testman::all_tests();
	}
	else if (cmd == "forward")
	{
		std::vector<float> inputs;
		inputs.resize(arguments.size() - 1);

		for (size_t i = 0; i < inputs.size(); i++)
		{
			inputs[i] = std::stof(arguments[i + 1]);
			std::cout << inputs[i] << "   ";
		}
		std::cout << "\n";

		neutron::network neural_network = neutron::network::read_network(arguments[0]);
		neutron::network::output output = neural_network.forward_pass(inputs);
	}
	else if (cmd == "read")
	{
		neutron::network neural_network = neutron::network::read_network(arguments[0]);
		neural_network.output_network();
	}

	auto end = std::chrono::high_resolution_clock::now();
	if (bench)
	{
		std::chrono::duration<double, std::milli> elapsed = end - start;
		std::cout << "-bench: " << elapsed.count() << "ms" << std::endl;
	}
	return 0;
}
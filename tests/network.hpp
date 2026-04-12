#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdint>
#include <vector>
#include "../neutron.hpp"

namespace network
{
    namespace fs = std::filesystem;
    inline static char test_file_name[] = "network_test_file.binary";

    bool create_network()
    {
        std::vector<uint32_t> layers = {2, 3, 2};
        neutron::network new_network = neutron::network(layers);

        /*
            Expected data:
            config data[0] = {2} (config version 1 says there is only one config uint32_t, and it is responsible for input size)
            layers should have 2 neutron::network::layer instances (layers[0] is simply for input size, not actually a layer), each with filled out input and output sizes, and proper amounts of weights and biases.

            layer 0:
            weights: 6
            biases: 3
            input size: 2
            output size: 3

            layer 1:
            weights: 6
            biases: 2
            input size: 3
            output size: 2
        */

        if (new_network.config_data.size() != 1)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: config metadata size isn't as expected.\n";
            return false;
        }
        if (new_network.config_data[0] != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: config metadata isn't as expected.\n";
            return false;
        }
        if (new_network.layers.size() != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: amount of layers isn't as expected.\n";
            return false;
        }

        if (new_network.layers[0].weights.size() != 6)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: amount of weights isn't as expected.\n";
            return false;
        }
        if (new_network.layers[0].biases.size() != 3)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: amount of biases isn't as expected.\n";
            return false;
        }
        if (new_network.layers[0].input_size != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: input size isn't as expected.\n";
            return false;
        }
        if (new_network.layers[0].output_size != 3)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: output size isn't as expected.\n";
            return false;
        }

        if (new_network.layers[1].weights.size() != 6)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: amount of weights isn't as expected.\n";
            return false;
        }
        if (new_network.layers[1].biases.size() != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: amount of biases isn't as expected.\n";
            return false;
        }
        if (new_network.layers[1].input_size != 3)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: input size isn't as expected.\n";
            return false;
        }
        if (new_network.layers[1].output_size != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: create_network: output size isn't as expected.\n";
            return false;
        }

        return true;
    }

    neutron::network test_network;
    inline bool save_network()
    {
        std::vector<uint32_t> layers = {2, 3, 2};
        test_network = neutron::network(layers);

        // Destroy any previous file (if any, not really critical)
        try
        {
            fs::remove(test_file_name);
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: failed to delete previous file, error message: \"" << e.what() << "\"" << std::endl;
        }

        test_network.save_network(test_file_name);

        std::fstream file(test_file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: failed to open \"" << test_file_name << "\".\n";
            return false;
        }

        /*
            version: anything nonzero cuz I'm lazy
            blocks: 4
            config size: 1
            config data 1: 2 (input size)
            block 0: size should be 6 uint32_t's * 4 bytes = 24 bytes
            block 1: size should be 3 uint32_t's * 4 bytes = 12 bytes
            block 2: size should be 6 uint32_t's * 4 bytes = 24 bytes
            block 3: size should be 2 uint32_t's * 4 bytes = 8 bytes
        */

        neutron::binary::file_metadata metadata = neutron::binary::read_metadata(file);

        std::vector<float> block0 = neutron::binary::read_block(test_file_name, 0);
        std::vector<float> block1 = neutron::binary::read_block(test_file_name, 1);
        std::vector<float> block2 = neutron::binary::read_block(test_file_name, 2);
        std::vector<float> block3 = neutron::binary::read_block(test_file_name, 3);

        // Version
        if (metadata.version == 0)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: version is zero.\n";
            return false;
        }

        // Blocks
        if (metadata.blocks != 4)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: blocks isn't as expected.\n";
            return false;
        }

        // Block sizes
        if (metadata.block_sizes != std::vector<uint32_t>{24, 12, 24, 8})
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: block sizes isn't as expected.\n";
            return false;
        }

        // Config size
        if (metadata.config_size != 1)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: config_size isn't as expected.\n";
            return false;
        }

        // Config data
        if (metadata.config_data != std::vector<uint32_t>{2})
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: config_data isn't as expected.\n";
            return false;
        }

        // Block 0
        if (block0.size() != 6)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: block 0 size isn't as expected.\n";
            return false;
        }

        // Block 1
        if (block1.size() != 3)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: block 1 size isn't as expected.\n";
            return false;
        }

        // Block 2
        if (block2.size() != 6)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: block 2 size isn't as expected.\n";
            return false;
        }

        // Block 3
        if (block3.size() != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m network: save_network: block 4 size isn't as expected.\n";
            return false;
        }

        file.close();
        return true;
    }

    /*
    bool read_network()
    {}
    */

    inline bool network()
    {
        bool success = true;
        if (!create_network())
        {
            std::cout << "\033[31m[ FAILED ]\033[0m network: create_network()\n";
            std::cout << "\033[31m[ FATAL ]\033[0m network: create_network() was required for further tests, quitting network test.\n";
            success = false;
        }
        else
        {
            std::cout << "\033[32m[ PASSED ]\033[0m network: create_network()\n";
            if (!save_network())
            {
                std::cout << "\033[31m[ FAILED ]\033[0m network: save_network()\n";
                std::cout << "\033[31m[ FATAL ]\033[0m network: save_network() was required for further tests, quitting network test.\n";
                success = false;
            }
            else
            {
                std::cout << "\033[32m[ PASSED ]\033[0m network: save_network()\n";
            }
        }

        if (!success)
        {
            std::cout << "\033[33m[ NOTICE ]\033[0m binary: \033[1msome tests failed, check the binary test file \"" << test_file_name << "\" at the working directory.\033[0m" << std::endl;
        }
        else
        {
            try
            {
                fs::remove(test_file_name);
            }
            catch (const fs::filesystem_error &e)
            {
                std::cerr << "\033[31m[ ERROR ]\033[0m binary: failed to delete file, error message: \"" << e.what() << "\"" << std::endl;
            }
        }

        return success;
    }
}

#endif
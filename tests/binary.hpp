#ifndef BINARY_HPP
#define BINARY_HPP

#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>
#include <cstdint>
#include "../neutron.hpp"

namespace binary
{
    namespace fs = std::filesystem;
    static char test_file_name[] = "binary_test_file.binary";

    inline bool insert_bytes()
    {
        uint32_t fails = 0;

        // Destroy any previous file (if any, not really critical)
        try
        {
            fs::remove(test_file_name);
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: insert_bytes: failed to delete previous file, error message: \"" << e.what() << "\"" << std::endl;
        }

        // Create a new binary to test
        neutron::binary::new_bin(test_file_name);

        std::fstream file(test_file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: insert_bytes: failed to open \"" << test_file_name << "\".\n";
            fails++;
        }

        std::vector<uint32_t> data = {1, 6, 4};
        neutron::binary::insert_bytes(test_file_name, file, 0, 0, reinterpret_cast<char *>(data.data()), data.size() * sizeof(uint32_t));

        // Overwrite 4 bytes with 8 bytes
        std::vector<uint32_t> to_overwrite = {2, 3};
        std::vector<uint32_t> expected_overwrite_results = {1, 2, 3, 4};
        std::vector<uint32_t> overwrite_results = {};
        overwrite_results.resize(expected_overwrite_results.size());
        neutron::binary::insert_bytes(test_file_name, file, sizeof(uint32_t), sizeof(uint32_t), reinterpret_cast<char *>(to_overwrite.data()), to_overwrite.size() * sizeof(uint32_t));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(overwrite_results.data()), expected_overwrite_results.size() * sizeof(uint32_t));
        if (overwrite_results != expected_overwrite_results)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: insert_bytes: overwrite test results did not match expectations\n";
            fails++;
        }

        // Append 4 bytes to the end of a file
        std::vector<uint32_t> to_append = {5};
        std::vector<uint32_t> expected_append_results = {1, 2, 3, 4, 5};
        std::vector<uint32_t> append_results = {};
        append_results.resize(expected_append_results.size());
        neutron::binary::insert_bytes(test_file_name, file, 4 * sizeof(uint32_t), 0, reinterpret_cast<char *>(to_append.data()), to_append.size() * sizeof(uint32_t));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(append_results.data()), expected_append_results.size() * sizeof(uint32_t));
        if (append_results != expected_append_results)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: insert_bytes: append test results did not match expectations\n";
            fails++;
        }

        // Overwrite 16 bytes with 8 bytes
        std::vector<uint32_t> to_smaller_overwrite = {6, 7};
        std::vector<uint32_t> expected_smaller_overwrite_results = {1, 6, 7};
        std::vector<uint32_t> smaller_overwrite_results = {};
        smaller_overwrite_results.resize(expected_smaller_overwrite_results.size());
        neutron::binary::insert_bytes(test_file_name, file, sizeof(uint32_t), 4 * sizeof(uint32_t), reinterpret_cast<char *>(to_smaller_overwrite.data()), to_smaller_overwrite.size() * sizeof(uint32_t));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(smaller_overwrite_results.data()), expected_smaller_overwrite_results.size() * sizeof(uint32_t));
        if (smaller_overwrite_results != expected_smaller_overwrite_results)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: insert_bytes: smaller overwrite test results did not match expectations\n";
            fails++;
        }

        // Insert 16 bytes
        std::vector<uint32_t> to_insert = {2, 3, 4, 5};
        std::vector<uint32_t> expected_insert_results = {1, 2, 3, 4, 5, 6, 7};
        std::vector<uint32_t> insert_results = {};
        insert_results.resize(expected_insert_results.size());
        neutron::binary::insert_bytes(test_file_name, file, sizeof(uint32_t), 0, reinterpret_cast<char *>(to_insert.data()), to_insert.size() * sizeof(uint32_t));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(insert_results.data()), expected_insert_results.size() * sizeof(uint32_t));
        if (insert_results != expected_insert_results)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: insert_bytes: insert test results did not match expectations\n";
            fails++;
        }

        file.close();
        return (fails == 0);
    }
    inline bool new_bin()
    {
        uint32_t fails = 0;

        // Destroy any previous file (if any, not really critical)
        try
        {
            fs::remove(test_file_name);
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: failed to delete previous file, error message: \"" << e.what() << "\"" << std::endl;
        }

        // Create a new binary to test
        neutron::binary::new_bin(test_file_name);

        /*
            Expected structure of a blank file:
            version > 0 (I don't wanna update this after each new binary version lol)
            blocks == 0
            config_size == 0
        */
        std::fstream file(test_file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: failed to open \"" << test_file_name << "\".\n";
            fails++;
        }

        uint32_t version = 0;
        uint32_t blocks = 0;
        uint32_t config_size = 0;

        file.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: error reading version metadata.\n";
            fails++;
        }

        file.read(reinterpret_cast<char *>(&blocks), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: error reading blocks metadata.\n";
            fails++;
        }

        file.read(reinterpret_cast<char *>(&config_size), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: error reading config_size metadata.\n";
            fails++;
        }

        if (version == 0)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: version metadata is zero.\n";
            fails++;
        }
        else
        {
            std::cout << "\033[33m[ NOTICE ]\033[0m binary: new_bin: \033[1mtest binary version is: \"" << version << "\" make sure that's as expected!\033[0m\n";
        }
        if (blocks != 0)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: blocks metadata isn't zero.\n";
            fails++;
        }
        if (config_size != 0)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: new_bin: config_size metadata isn't zero.\n";
            fails++;
        }
        file.close();
        return (fails == 0);
    }
    inline bool write_block()
    {
        uint32_t fails = 0;

        std::fstream file(test_file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: failed to open \"" << test_file_name << "\".\n";
            fails++;
        }

        std::vector<float> block0 = {999.0f};
        neutron::binary::write_block(test_file_name, 0, block0);
        std::vector<float> block1 = {4.0f, 2.0f, 1.4142135f};
        neutron::binary::write_block(test_file_name, 1, block1);

        /*
            Expected structure after these modifications:
            version: doesn't matter
            blocks: 2
            block_sizes: 4, 12
            config_size: 0, we haven't modified it.
            config entries: none, should go straight to 999.0f.
            block 0: 999.0f
            block 1: 4.0f, 2.0f, 1.4142135f
        */

        uint32_t blocks = 0;
        std::vector<uint32_t> block_sizes = {};
        uint32_t config_size = 0;
        std::vector<uint32_t> config_data = {};
        std::vector<float> block0_read = {};
        std::vector<float> block1_read = {};

        file.seekg(sizeof(uint32_t)); // skip past version

        // blocks
        file.read(reinterpret_cast<char *>(&blocks), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: error reading blocks.\n";
            fails++;
        }
        if (blocks != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: blocks metadata was incorrectly written\n";
            fails++;
        }

        // block_sizes
        block_sizes.resize(blocks);
        file.read(reinterpret_cast<char *>(block_sizes.data()), sizeof(uint32_t) * blocks);
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: error reading block_sizes.\n";
            fails++;
        }
        if (block_sizes.size() != 2)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: block_sizes metadata has the wrong amount of size entries\n";
            fails++;
        }
        if (block_sizes[0] != block0.size() * sizeof(float) || block_sizes[1] != block1.size() * sizeof(float))
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: block_sizes metadata has the wrong sizes\n";
            fails++;
        }

        // config_size
        file.read(reinterpret_cast<char *>(&config_size), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: error reading config_size.\n";
            fails++;
        }
        if (config_size != 0)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: config_size metadata was incorrectly written\n";
            fails++;
        }

        // config_data
        config_data.resize(config_size);
        file.read(reinterpret_cast<char *>(config_data.data()), sizeof(uint32_t) * config_size);
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: error reading config_data.";
            fails++;
        }
        if (config_data.size() != 0)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: config_data metadata should have zero entries\n";
            fails++;
        }

        // reading blocks
        block0_read.resize(block_sizes[0] / sizeof(float));
        file.read(reinterpret_cast<char *>(block0_read.data()), block_sizes[0]);
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: error reading block 0\n";
            fails++;
        }

        block1_read.resize(block_sizes[1] / sizeof(float));
        file.read(reinterpret_cast<char *>(block1_read.data()), block_sizes[1]);
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: error reading block 1\n";
            fails++;
        }

        if (block0 != block0_read)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: block 0 and the block 0 read from the binary did not match.\n";
            fails++;
        }
        if (block1 != block1_read)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: write_block: block 1 and the block 1 read from the binary did not match.\n";
            fails++;
        }

        file.close();
        return (fails == 0);
    }
    inline bool read_block()
    {
        uint32_t fails = 0;

        /*
            Expected read results:
            block 0: 999.0f
            block 1: 4, 2, 1.4142135f
        */

        std::vector<float> block0 = {999.0f};
        std::vector<float> block1 = {4.0f, 2.0f, 1.4142135f};

        std::vector<float> block0_read = neutron::binary::read_block(test_file_name, 0);
        std::vector<float> block1_read = neutron::binary::read_block(test_file_name, 1);

        if (block0 != block0_read)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_block: what was expected from block 0 and the block 0 read from the binary did not match.\n";
            fails++;
        }
        if (block1 != block1_read)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_block: what was expected from block 1 and the block 1 read from the binary did not match.\n";
            fails++;
        }
        return (fails == 0);
    }
    inline bool read_metadata()
    {
        uint32_t fails = 0;

        std::fstream file(test_file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: failed to open \"" << test_file_name << "\".\n";
            fails++;
        }

        // add one 4 byte config
        uint32_t blocks = 0;
        std::vector<uint32_t> block_sizes = {};
        file.seekg(sizeof(uint32_t));

        // blocks
        file.read(reinterpret_cast<char *>(&blocks), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: error reading blocks.\n";
            fails++;
        }

        // block_sizes
        block_sizes.resize(blocks);
        file.read(reinterpret_cast<char *>(block_sizes.data()), sizeof(uint32_t) * blocks);
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: error reading block_sizes.\n";
            fails++;
        }

        /*size_t sum = sizeof(uint32_t) * (3 + blocks);
        uint32_t one = 1;
        neutron::binary::insert_bytes(test_file_name, file, sum, sizeof(uint32_t), reinterpret_cast<char *>(&one), sizeof(uint32_t));	// config_size
        neutron::binary::insert_bytes(test_file_name, file, sum + sizeof(uint32_t), 0, reinterpret_cast<char *>(&one), sizeof(uint32_t)); // config_data*/

        neutron::binary::file_metadata metadata = neutron::binary::read_metadata(file);

        uint32_t version = 0;
        uint32_t config_size = 0;
        std::vector<uint32_t> config_data = {};

        file.seekg(0, std::ios::beg);

        // version
        file.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: error reading blocks.\n";
            fails++;
        }
        if (version != metadata.version)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: version metadata mismatch\n";
            fails++;
        }

        // blocks
        file.read(reinterpret_cast<char *>(&blocks), sizeof(uint32_t));
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: error reading blocks.\n";
            fails++;
        }
        if (blocks != metadata.blocks)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: blocks metadata mismatch\n";
            fails++;
        }

        // block_sizes
        block_sizes.resize(blocks);
        file.read(reinterpret_cast<char *>(block_sizes.data()), sizeof(uint32_t) * blocks);
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: error reading block_sizes.\n";
            fails++;
        }
        if (block_sizes != metadata.block_sizes)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: block_sizes metadata mismatch\n";
            fails++;
        }

        // config_size
        file.read(reinterpret_cast<char *>(&config_size), sizeof(uint32_t));

        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: error reading config_size.\n";
            fails++;
        }
        if (config_size != metadata.config_size)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: config_size metadata mismatch\n";
            fails++;
        }

        // config_data
        config_data.resize(config_size);
        file.read(reinterpret_cast<char *>(config_data.data()), sizeof(uint32_t) * config_size);
        if (!file)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: error reading config_data.\n";
            fails++;
        }
        if (config_data != metadata.config_data)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: read_metadata: config_data metadata mismatch\n";
            fails++;
        }

        file.close();
        return (fails == 0);
    }
    inline bool overwrite_config()
    {
        uint32_t fails = 0;

        std::fstream file(test_file_name, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: overwrite_config: failed to open \"" << test_file_name << "\".\n";
            fails++;
        }

        std::vector<uint32_t> data = {1111};

        neutron::binary::overwrite_config(test_file_name, file, data);

        neutron::binary::file_metadata metadata = neutron::binary::read_metadata(file);

        if (metadata.config_size != 1)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: overwrite_config: config_size metadata mismatch\n";
            fails++;
        }
        if (metadata.config_data != data)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: overwrite_config: config_data metadata mismatch\n";
            fails++;
        }

        std::vector<float> block0 = {999.0f};
        std::vector<float> block0_read = neutron::binary::read_block(test_file_name, 0);
        if (block0 != block0_read)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: overwrite_config: block 1 mismatch; config data is likely taking up more bytes than expected.\n";
            fails++;
        }

        file.close();
        return (fails == 0);
    }

    inline bool binary()
    {
        bool success = true;
        bool test_result = false;

        // insert_bytes
        try
        {
            test_result = insert_bytes();
        }
        catch (const std::exception &e)
        {
            std::cerr << "\033[31m[ ERROR ]\033[0m binary: critical error caught: " << e.what() << "\n";
            test_result = false;
        }
        if (!test_result)
        {
            std::cout << "\033[31m[ FAILED ]\033[0m binary: insert_bytes()\n";
            std::cout << "\033[31m[ FATAL ]\033[0m binary: insert_bytes() was required for further tests, quitting binary test.\n";
            success = false;
        }
        else
        {
            std::cout << "\033[32m[ PASSED ]\033[0m binary: insert_bytes()\n";

            // new_bin
            try
            {
                test_result = new_bin();
            }
            catch (const std::exception &e)
            {
                std::cerr << "\033[31m[ ERROR ]\033[0m binary: critical error caught: " << e.what() << "\n";
                test_result = false;
            }
            if (!test_result)
            {
                std::cout << "\033[31m[ FAILED ]\033[0m binary: new_bin()\n";
                std::cout << "\033[31m[ FATAL ]\033[0m binary: new_bin() was required for further tests, quitting binary test.\n";
                success = false;
            }
            else
            {
                std::cout << "\033[32m[ PASSED ]\033[0m binary: new_bin()\n";

                // write_block
                try
                {
                    test_result = write_block();
                }
                catch (const std::exception &e)
                {
                    std::cerr << "\033[31m[ ERROR ]\033[0m binary: critical error caught: " << e.what() << "\n";
                    test_result = false;
                }
                if (!test_result)
                {
                    std::cout << "\033[31m[ FAILED ]\033[0m binary: write_block()\n";
                    std::cout << "\033[31m[ FATAL ]\033[0m binary: write_block() was required for further tests, quitting binary test.\n";
                    success = false;
                }
                else
                {
                    std::cout << "\033[32m[ PASSED ]\033[0m binary: write_block()\n";

                    // read_block
                    try
                    {
                        test_result = read_block();
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "\033[31m[ ERROR ]\033[0m binary: critical error caught: " << e.what() << "\n";
                        test_result = false;
                    }
                    if (!test_result)
                    {
                        std::cout << "\033[31m[ FAILED ]\033[0m binary: read_block()\n";
                        success = false;
                    }
                    else
                    {
                        std::cout << "\033[32m[ PASSED ]\033[0m binary: read_block()\n";
                    }

                    // read_metadata
                    try
                    {
                        test_result = read_metadata();
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "\033[31m[ ERROR ]\033[0m binary: critical error caught: " << e.what() << "\n";
                        test_result = false;
                    }
                    if (!test_result)
                    {
                        std::cout << "\033[31m[ FAILED ]\033[0m binary: read_metadata()\n";
                        std::cout << "\033[31m[ FATAL ]\033[0m binary: read_metadata() was required for further tests, quitting binary test.\n";
                        success = false;
                    }
                    else
                    {
                        std::cout << "\033[32m[ PASSED ]\033[0m binary: read_metadata()\n";

                        // overwrite_config
                        try
                        {
                            test_result = overwrite_config();
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << "\033[31m[ ERROR ]\033[0m binary: critical error caught: " << e.what() << "\n";
                            test_result = false;
                        }
                        if (!test_result)
                        {
                            std::cout << "\033[31m[ FAILED ]\033[0m binary: overwrite_config()\n";
                            success = false;
                        }
                        else
                        {
                            std::cout << "\033[32m[ PASSED ]\033[0m binary: overwrite_config()\n";
                        }
                    }
                }
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
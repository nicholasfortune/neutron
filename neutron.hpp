#ifndef NEUTRON_HPP
#define NEUTRON_HPP

#include <cstdint>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <filesystem>
#include <random>
#include <string_view>
#include <cmath>

namespace neutron
{
	namespace binary
	{
		struct file_metadata
		{
			uint32_t version;
			uint32_t blocks;
			std::vector<uint32_t> block_sizes;
			uint32_t config_size;
			std::vector<uint32_t> config_data;
		};

		inline void insert_bytes(char *location, std::fstream &file, std::streampos position, size_t old_data_size, const char *data, size_t data_size)
		{
			if (!file.is_open())
			{
				std::cerr << "insert_bytes: file not open\n";
				return;
			}

			// Move to end to get file size
			file.seekg(0, std::ios::end);
			std::streampos sp = file.tellg();
			if (sp == -1)
			{
				std::cerr << "insert_bytes: tellg failed\n";
				return;
			}

			size_t file_size = static_cast<size_t>(sp);

			if (position < 0 || static_cast<size_t>(position) + old_data_size > file_size)
			{
				std::cerr << "insert_bytes: position + old_data_size exceeds file size\n";
				return;
			}

			// Calculate tail size
			size_t tail_size = file_size - (static_cast<size_t>(position) + old_data_size);

			// Read tail if there is any
			std::vector<char> tail;
			if (tail_size > 0)
			{
				tail.resize(tail_size);
				file.seekg(position + static_cast<std::streamoff>(old_data_size), std::ios::beg);
				file.read(tail.data(), static_cast<std::streamsize>(tail_size));
				if (!file)
				{
					std::cerr << "insert_bytes: read tail failed\n";
					return;
				}
			}

			// Write new data
			file.seekp(position, std::ios::beg);
			if (data_size > 0)
			{
				file.write(data, static_cast<std::streamsize>(data_size));
				if (!file)
				{
					std::cerr << "insert_bytes: write data failed\n";
					return;
				}
			}

			// Write tail
			if (tail_size > 0)
			{
				file.write(tail.data(), static_cast<std::streamsize>(tail_size));
				if (!file)
				{
					std::cerr << "insert_bytes: write tail failed\n";
					return;
				}
			}

			// Resize file if new file is smaller or larger
			size_t new_file_size = file_size - old_data_size + data_size;
			if (new_file_size != file_size)
			{
				file.flush();
				std::error_code ec;
				std::filesystem::resize_file(location, new_file_size, ec);
				if (ec)
				{
					std::cerr << "insert_bytes: resize_file failed: " << ec.message() << '\n';
					return;
				}
			}

			file.clear(); // Reset any flags
			return;
		}
		inline file_metadata read_metadata(std::fstream &file)
		{
			if (!file.is_open())
			{
				std::cerr << "read_metadata: failed to open provided file.\n";
				return {};
			}

			file.seekg(0, std::ios::beg);

			// Version
			uint32_t version;
			file.read(reinterpret_cast<char *>(&version), sizeof(version));
			if (!file)
			{
				std::cerr << "read_metadata: error getting version metadata.\n";
				return neutron::binary::file_metadata{};
			}

			// Blocks
			uint32_t blocks;
			file.read(reinterpret_cast<char *>(&blocks), sizeof(blocks));
			if (!file)
			{
				std::cerr << "read_metadata: error getting blocks metadata.\n";
				return neutron::binary::file_metadata{};
			}

			// Block sizes
			std::vector<uint32_t> block_sizes(blocks);
			file.read(reinterpret_cast<char *>(block_sizes.data()), blocks * sizeof(uint32_t));
			if (!file)
			{
				std::cerr << "read_metadata: error getting block sizes metadata.\n";
				return neutron::binary::file_metadata{};
			}

			// Config size
			uint32_t config_size;
			file.read(reinterpret_cast<char *>(&config_size), sizeof(config_size));
			if (!file)
			{
				std::cerr << "read_metadata: error getting config_size metadata.\n";
				return neutron::binary::file_metadata{};
			}

			// Config data
			std::vector<uint32_t> config_data(config_size);
			file.read(reinterpret_cast<char *>(config_data.data()), config_size * sizeof(uint32_t));
			if (!file)
			{
				std::cerr << "read_metadata: error getting config_data metadata.\n";
				return neutron::binary::file_metadata{};
			}

			return neutron::binary::file_metadata{version, blocks, block_sizes, config_size, config_data};
		}
		inline void overwrite_config(char *location, std::fstream &file, std::vector<uint32_t> config_data)
		{
			if (!file.is_open())
			{
				std::cerr << "overwrite_config: failed to open provided file.\n";
				return;
			}

			neutron::binary::file_metadata metadata = read_metadata(file);
			size_t sum = sizeof(uint32_t) * (2 + metadata.blocks); // Size of metadata minus config_size and config_data in bytes

			// Change config_size
			uint32_t value = static_cast<uint32_t>(config_data.size());
			file.seekg(sum);
			file.write(reinterpret_cast<const char *>(&value), sizeof(uint32_t));

			// Change config_data
			insert_bytes(location, file, sum + sizeof(uint32_t), sizeof(uint32_t) * metadata.config_size, reinterpret_cast<const char *>(config_data.data()), sizeof(uint32_t) * config_data.size());

			file.seekg(0, std::ios::beg);
		}
		inline std::vector<float> read_block(char *location, uint32_t block)
		{
			// Open file
			std::fstream file(location, std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "read_block: failed to open \"" << location << "\".\n";
				return {};
			}

			neutron::binary::file_metadata file_metadata = read_metadata(file);
			uint32_t version = file_metadata.version;
			uint32_t blocks = file_metadata.blocks;
			std::vector<uint32_t> block_sizes = file_metadata.block_sizes;
			uint32_t config_size = file_metadata.config_size;

			// Find the block the user wants
			if (block >= blocks)
			{
				std::cerr << "Block # requested is invalid.\n";
				return {};
			}

			// Sum the size of all blocks before the one the user wants
			size_t sum = sizeof(uint32_t) * (3 + blocks + config_size); // Size of metadata
			for (size_t i = 0; i < block; i++)
			{
				sum += block_sizes[i];
			}

			// Read the block
			size_t bytes = static_cast<size_t>(block_sizes[block]);
			if (bytes % sizeof(float) != 0)
			{
				std::cerr << "read_block: block size not aligned with type\n";
				return {};
			}

			std::vector<float> wanted_block(bytes / sizeof(float));

			file.seekg(sum, std::ios::beg);
			if (!file)
			{
				std::cerr << "read_block: seekg failed for block " << block << "\n";
				return {};
			}

			file.read(reinterpret_cast<char *>(wanted_block.data()), bytes);
			if (!file)
			{
				std::cerr << "read_block: error reading block " << block << "\n";
				return {};
			}

			return wanted_block;
		}
		inline void write_block(char *location, uint32_t block, std::vector<float> values)
		{
			// Open file
			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "write_block: failed to open \"" << location << "\".\n";
				return;
			}

			neutron::binary::file_metadata file_metadata = read_metadata(file);
			uint32_t version = file_metadata.version;
			uint32_t blocks = file_metadata.blocks;
			std::vector<uint32_t> block_sizes = file_metadata.block_sizes;
			uint32_t config_size = file_metadata.config_size;

			// Find the block the user wants
			size_t sum = sizeof(uint32_t) * (3 + blocks + config_size); // Size of metadata
			for (size_t i = 0; i < block && i < blocks; i++)
				sum += block_sizes[i];

			uint32_t size = static_cast<uint32_t>(values.size() * sizeof(float));

			// Write a new block
			if (block == blocks)
			{

				// Write new block at end
				file.seekp(sum, std::ios::beg);
				file.write(reinterpret_cast<const char *>(values.data()), size);

				// Append new block sizes entry
				insert_bytes(location, file, (2 + block) * sizeof(uint32_t), 0, reinterpret_cast<const char *>(&size), sizeof(uint32_t));

				// Increment blocks"
				blocks++;
				file.seekp(4, std::ios::beg);
				file.write(reinterpret_cast<const char *>(&blocks), sizeof(uint32_t));
			}

			// Overwrite existing block
			else if (block < blocks)
			{

				insert_bytes(location, file, sum, block_sizes[block], reinterpret_cast<const char *>(values.data()), size);

				// Overwrite "block sizes" entry
				file.seekp(8 + block * sizeof(uint32_t), std::ios::beg);
				file.write(reinterpret_cast<const char *>(&size), sizeof(uint32_t));
			}

			// Invalid block index
			else
			{
				std::cerr << "write_block: invalid block index\n";
			}

			file.flush();
			file.close();
		}
		inline void new_bin(char *location)
		{
			std::ofstream create(location, std::ios::binary | std::ios::trunc);
			if (!create.is_open())
			{
				std::cerr << "new_bin: cannot create \"" << location << "\"\n";
				return;
			}

			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);

			if (!file.is_open())
			{
				std::cerr << "new_bin: failed to reopen \"" << location << "\"\n";
				return;
			}

			// version
			uint32_t version = 1;
			file.write(reinterpret_cast<char *>(&version), sizeof(version));
			if (!file)
			{
				std::cerr << "new_bin: error writing version\n";
				return;
			}

			// blocks
			uint32_t blocks = 0;
			file.write(reinterpret_cast<char *>(&blocks), sizeof(blocks));
			if (!file)
			{
				std::cerr << "new_bin: error writing blocks\n";
				return;
			}

			// config size
			uint32_t config_size = 0;
			file.write(reinterpret_cast<char *>(&config_size), sizeof(config_size));
			if (!file)
			{
				std::cerr << "new_bin: error writing config_size\n";
				return;
			}
		}
	}

	class network
	{
	public:
		struct layer
		{
			std::vector<float> weights;
			std::vector<float> biases;
			uint32_t input_size;
			uint32_t output_size;
		};
		struct output
		{
			std::vector<float> outputs;
			std::vector<float> activations;
			std::vector<float> pre_activations;
		};
		struct backprop_averages
		{
			std::vector<std::vector<std::vector<float>>> weights;
			std::vector<std::vector<std::vector<float>>> biases;
		};

		std::vector<layer> layers;
		std::vector<uint32_t> config_data;

		network() = default;

		/*
			Creates an initialized, untrained neural network with the amount of layers being the amount of items in an array, and each item's value being the amount of neurons in that layer and the first layer being excluded as the input size.
		*/
		inline network(std::vector<uint32_t> layer_sizes)
		{
			size_t length = layer_sizes.size();
			if (length < 2)
			{
				std::cerr << "create_network: provided network is too small\n";
			}

			std::mt19937 gen(std::random_device{}());
			config_data.resize(1);
			config_data[0] = layer_sizes[0]; // Set input size

			std::vector<layer> new_layers;

			if (layer_sizes[1] == 0)
			{
				std::cerr << "create_network: layer 0 has zero neurons\n";
			}
			for (size_t i = 1; i < length; i++)
			{
				size_t i_minus_one = i - 1;
				if (i_minus_one < 0)
				{
					i_minus_one = layer_sizes[0];
				} // Set to input size
				neutron::network::layer hidden_layer;
				hidden_layer.input_size = layer_sizes[i_minus_one];
				hidden_layer.output_size = layer_sizes[i];
				size_t weights = (size_t)layer_sizes[i_minus_one] * (size_t)layer_sizes[i];
				hidden_layer.weights.resize(weights);

				for (size_t v = 0; v < weights; v++)
				{
					hidden_layer.weights[v] = initialize_weight(layer_sizes[i_minus_one], gen);
				}
				hidden_layer.biases.resize(layer_sizes[i], 0.01f);
				new_layers.push_back(hidden_layer);
			}

			layers = new_layers;
		}

		/*
			Saves or creates the provided neural network in the provided location. Will overwrite without confirmation.
		*/
		inline void save_network(char *location)
		{
			if (layers.size() < 2)
			{
				std::cerr << "save_network: provided network is too small\n";
				return;
			}

			neutron::binary::new_bin(location);

			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);

			if (!file.is_open())
			{
				std::cerr << "save_network: failed to open \"" << location << "\"\n";
				return;
			}

			neutron::binary::overwrite_config(location, file, config_data);

			size_t block = 0;
			for (size_t i = 0; i < layers.size(); i++)
			{
				if (layers[i].biases.size() == 0)
				{
					std::cerr << "save_network: layer " << i << "'s # of weights is 0\n";
					return;
				}
				neutron::binary::write_block(location, block, layers[i].weights);
				block++;
				if (layers[i].weights.size() == 0)
				{
					std::cerr << "save_network: layer " << i << "'s # of biases is 0\n";
					return;
				}
				neutron::binary::write_block(location, block, layers[i].biases);
				block++;
			}
		}

		/*
			Reads the neural network at a given location and returns it in the "neutron::network" struct.
		*/
		inline static network read_network(char *location)
		{
			std::fstream file(location, std::ios::in | std::ios::binary);
			neutron::binary::file_metadata file_metadata = neutron::binary::read_metadata(file);
			neutron::network new_network = network();

			size_t pointer = 0;
			// Loop through layers
			for (size_t layer = 0; layer < (file_metadata.blocks / 2); layer++)
			{
				new_network.layers[layer].biases = neutron::binary::read_block(location, pointer);
				pointer++;
				new_network.layers[layer].weights = neutron::binary::read_block(location, pointer);
				pointer++;
			}
			return new_network;
		}

		/*
			Passes inputs through a given neural network and returns the outputs.
		*/
		inline output forward_pass(std::vector<float> inputs)
		{
			if (layers.empty())
			{
				std::cerr << "forward_pass: network has no layers\n";
				return neutron::network::output{};
			}
			if (inputs.size() != layers[0].input_size)
			{
				std::cerr << "forward_pass: inputs do not match that of the provided neural network\n";
				return neutron::network::output{};
			}

			// Initialize the forward pass's outputs
			neutron::network::output fp_output;

			// Starting activations are just the inputs
			fp_output.outputs = inputs;

			// Loop through the layers
			for (size_t i = 0; i < layers.size(); i++)
			{
				neutron::network::layer layer = layers[i];

				// Allocate space for output activations of this layer
				std::vector<float> next_activations(layer.output_size, 0.0f);

				// Loop through the neurons
				for (size_t j = 0; j < layer.output_size; j++)
				{

					// Start the sum with the bias
					float sum = layer.biases[j];

					// Add each input * their respective weight
					for (size_t k = 0; k < layer.input_size; k++)
					{
						sum += fp_output.outputs[k] * layer.weights[j * layer.input_size + k]; // Flat array
					}

					// Save pre-activation sum
					fp_output.pre_activations.push_back(sum);

					// Activation function
					next_activations[j] = activation_function(sum);

					// Save activation
					fp_output.activations.push_back(next_activations[j]);
				}

				// These outputs become inputs for the next layer
				fp_output.outputs = std::move(next_activations);
			}

			return fp_output;
		}

		/*
			Prints out the contents of the provided neural network.
		*/
		inline void output_network()
		{
			for (int i = 0; i < layers.size(); i++)
			{
				int digits = (i == 0) ? 1 : (int)std::log10(abs(i)) + 1;
				std::cout << i;
				for (int i = 0; i < 4 - digits; ++i)
					std::cout << ' ';

				std::cout << "weights:    ";
				for (size_t v = 0; v < layers[i].weights.size(); v++)
				{
					std::cout << layers[i].weights[v] << "  ";
				}
				std::cout << std::endl;
				std::cout << "    biases:     ";
				for (size_t v = 0; v < layers[i].biases.size(); v++)
				{
					std::cout << layers[i].biases[v] << "  ";
				}
				std::cout << std::endl
						  << std::endl;
			}
		}

	private:
		inline float initialize_weight(uint32_t fan_in, std::mt19937 &gen)
		{
			return std::normal_distribution<float>(0.0f, std::sqrt(2.0f / fan_in))(gen);
		}
		inline float activation_function(float input)
		{
			return std::max(0.0f, input);
		}
		inline float activation_function_derivative(float input)
		{
			if (input > 0)
			{
				return 1.0f;
			}
			else
			{
				return 0.0f;
			}
		}
		inline float loss_function(float y_hat, float y)
		{
			return ((y_hat - y) * (y_hat - y)) / 2;
		}
		inline float gradient(float loss, float activation, float pre_activation)
		{
			return 0.0f;
		}
	};
}

#endif
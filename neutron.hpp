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

	namespace helper
	{
		float initialize_weight(uint32_t fan_in, std::mt19937 &gen)
		{
			return std::normal_distribution<float>(0.0f, std::sqrt(2.0f / fan_in))(gen);
		}
		float activation_function(float input)
		{
			return std::max(0.0f, input);
		}
		float activation_function_derivative(float input)
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
		float loss_function(float y_hat, float y)
		{
			return ((y_hat - y) * (y_hat - y)) / 2;
		}
		float loss_function_gradient(float y_hat, float y, float activation_derivative_of_z)
		{
			return (y_hat - y) * activation_derivative_of_z;
		}
		class matrix
		{
		public:
			uint32_t rows, columns;
			std::vector<float> data;

			inline matrix operator-(const matrix &b) const
			{
				if (this->rows != b.rows || this->columns != b.columns)
				{
					std::cerr << "matrix: subtraction: matrices must have the same dimensions\n";
					return matrix{};
				}

				matrix result(rows, columns);
				for (size_t i = 0; i < data.size(); ++i)
				{
					result.data[i] = this->data[i] - b.data[i];
				}
				return result;
			}

			matrix() = default;

			// Makes a new matrix with the given dimensions
			matrix(uint32_t r, uint32_t c) : rows(r), columns(c), data(r * c, 0.0f) {}

			// Makes a new matrix with the given dimensions from a given vector
			matrix(uint32_t r, uint32_t c, std::vector<float> vector) : rows(r), columns(c), data(r * c, 0.0f)
			{
				if (vector.size() != r * c)
				{
					std::cerr << "matrix: provided vector array does not match the size of the matrix\n";
					return;
				}

				for (uint32_t i = 0; i < data.size(); i++)
					data[i] = vector[i];
			}

			// Returns the value of a coordinate
			float get(uint32_t r, uint32_t c) const { return data[r * columns + c]; }

			// Sets the value of a coordinate
			void set(uint32_t r, uint32_t c, float value) { data[r * columns + c] = value; }

			// Returns a transposed copy of a given matrix
			matrix transpose() const
			{
				matrix result(columns, rows);
				for (uint32_t r = 0; r < rows; r++)
				{
					for (uint32_t c = 0; c < columns; c++)
					{
						result.set(c, r, this->get(r, c));
					}
				}
				return result;
			}

			// Returns the dot product of two matrices
			matrix dot(const matrix &b) const
			{
				if (this->columns != b.rows)
				{
					std::cerr << "matrix: dot: matrices' inner dimensions do not match\n";
					return matrix{};
				}
				matrix result(this->rows, b.columns);
				for (uint32_t i = 0; i < this->rows; i++)
				{
					for (uint32_t j = 0; j < b.columns; j++)
					{
						float sum = 0.0f;
						for (uint32_t k = 0; k < this->columns; k++)
						{
							sum += this->get(i, k) * b.get(k, j);
						}
						result.set(i, j, sum);
					}
				}
				return result;
			}

			// Returns the element-wise product of two identically sized matrices
			matrix element(const matrix &b) const
			{
				if (this->rows != b.rows || this->columns != b.columns)
				{
					std::cerr << "matrix: element: matrices must have identical dimensions\n";
					return matrix{};
				}
				matrix result(rows, columns);
				for (size_t i = 0; i < data.size(); i++)
				{
					result.data[i] = this->data[i] * b.data[i];
				}
				return result;
			}

			matrix map(float (*func)(float)) const
			{
				matrix result(rows, columns);
				for (size_t i = 0; i < data.size(); ++i)
				{
					result.data[i] = func(this->data[i]);
				}
				return result;
			}
		};
	}

	namespace binary
	{
		struct header
		{
			uint32_t version;
			uint32_t blocks;
			std::vector<uint32_t> block_sizes;
			uint32_t config_size;
			std::vector<uint32_t> config_data;
		};

		// Versatile insert/overwrite function for writing data to filesystems
		void insert_bytes(char *location, std::streampos position, size_t old_data_size, const char *data, size_t data_size)
		{
			// Open file
			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "read_block: failed to open \"" << location << "\".\n";
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
		// Reads all metadata from a requested `.neu` file and provides it in the `neuron::binary::header` struct
		header read_metadata(char *location)
		{
			// Open file
			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "read_block: failed to open \"" << location << "\".\n";
				return neutron::binary::header{};
			}

			if (!file.is_open())
			{
				std::cerr << "read_metadata: failed to open provided file.\n";
				return neutron::binary::header{};
			}

			file.seekg(0, std::ios::beg);

			// Version
			uint32_t version;
			file.read(reinterpret_cast<char *>(&version), sizeof(version));
			if (!file)
			{
				std::cerr << "read_metadata: error getting version metadata.\n";
				return neutron::binary::header{};
			}

			// Blocks
			uint32_t blocks;
			file.read(reinterpret_cast<char *>(&blocks), sizeof(blocks));
			if (!file)
			{
				std::cerr << "read_metadata: error getting blocks metadata.\n";
				return neutron::binary::header{};
			}

			// Block sizes
			std::vector<uint32_t> block_sizes(blocks);
			file.read(reinterpret_cast<char *>(block_sizes.data()), blocks * sizeof(uint32_t));
			if (!file)
			{
				std::cerr << "read_metadata: error getting block sizes metadata.\n";
				return neutron::binary::header{};
			}

			// Config size
			uint32_t config_size;
			file.read(reinterpret_cast<char *>(&config_size), sizeof(config_size));
			if (!file)
			{
				std::cerr << "read_metadata: error getting config_size metadata.\n";
				return neutron::binary::header{};
			}

			// Config data
			std::vector<uint32_t> config_data(config_size);
			file.read(reinterpret_cast<char *>(config_data.data()), config_size * sizeof(uint32_t));
			if (!file)
			{
				std::cerr << "read_metadata: error getting config_data metadata.\n";
				return neutron::binary::header{};
			}

			return neutron::binary::header{version, blocks, block_sizes, config_size, config_data};
		}
		// Overwrites the full config of a `.neu` file with a provided array
		void overwrite_config(char *location, std::vector<uint32_t> config_data)
		{
			// Open file
			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "read_block: failed to open \"" << location << "\".\n";
				return;
			}

			neutron::binary::header metadata = read_metadata(location);
			size_t sum = sizeof(uint32_t) * (2 + metadata.blocks); // Size of metadata minus config_size and config_data in bytes

			// Change config_size
			uint32_t value = static_cast<uint32_t>(config_data.size());
			file.seekg(sum);
			file.write(reinterpret_cast<const char *>(&value), sizeof(uint32_t));

			// Change config_data
			insert_bytes(location, sum + sizeof(uint32_t), sizeof(uint32_t) * metadata.config_size, reinterpret_cast<const char *>(config_data.data()), sizeof(uint32_t) * config_data.size());

			file.seekg(0, std::ios::beg);
		}
		// Reads the floats at a requested block from the provided `.neu` file
		std::vector<float> read_block(char *location, uint32_t block)
		{
			// Open file
			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "read_block: failed to open \"" << location << "\".\n";
				return {};
			}

			neutron::binary::header header = read_metadata(location);
			uint32_t version = header.version;
			uint32_t blocks = header.blocks;
			std::vector<uint32_t> block_sizes = header.block_sizes;
			uint32_t config_size = header.config_size;

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
		// Writes an array of floats at the requested block to the provided `.neu` file
		void write_block(char *location, uint32_t block, std::vector<float> values)
		{
			// Open file
			std::fstream file(location, std::ios::in | std::ios::out | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "write_block: failed to open \"" << location << "\".\n";
				return;
			}

			neutron::binary::header header = read_metadata(location);
			uint32_t version = header.version;
			uint32_t blocks = header.blocks;
			std::vector<uint32_t> block_sizes = header.block_sizes;
			uint32_t config_size = header.config_size;

			// Find the block the user wants
			size_t sum = sizeof(uint32_t) * (4 + blocks + config_size); // Size of metadata
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
				insert_bytes(location, (2 + block) * sizeof(uint32_t), 0, reinterpret_cast<const char *>(&size), sizeof(uint32_t));

				// Increment blocks"
				blocks++;
				file.seekp(4, std::ios::beg);
				file.write(reinterpret_cast<const char *>(&blocks), sizeof(uint32_t));
			}

			// Overwrite existing block
			else if (block < blocks)
			{
				insert_bytes(location, sum, block_sizes[block], reinterpret_cast<const char *>(values.data()), size);

				// Overwrite "block sizes" entry
				file.seekp(2 * sizeof(uint32_t) + block * sizeof(uint32_t), std::ios::beg);
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
		// Creates an empty `.neu` header at the given location.
		void new_bin(char *location)
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
		struct backprop_gradients
		{
			std::vector<std::vector<float>> weights;
			std::vector<std::vector<float>> biases;
		};

		std::vector<layer> layers;
		std::vector<uint32_t> config_data;

		network() = default;
		// Creates an initialized, untrained neural network with the amount of layers being the amount of items in an array, and each item's value being the amount of neurons in that layer and the first layer being excluded as the input size.
		network(std::vector<uint32_t> layer_sizes)
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
					hidden_layer.weights[v] = helper::initialize_weight(layer_sizes[i_minus_one], gen);
				}
				hidden_layer.biases.resize(layer_sizes[i], 0.01f);
				new_layers.push_back(hidden_layer);
			}

			layers = new_layers;
		}
		// Saves or creates a neural network in the provided location. Will overwrite without confirmation.
		void save_network(char *location)
		{
			if (layers.size() < 2)
			{
				std::cerr << "save_network: provided network is too small\n";
				return;
			}

			neutron::binary::new_bin(location);

			neutron::binary::overwrite_config(location, config_data);

			size_t block = 0;
			for (size_t i = 0; i < layers.size(); i++)
			{
				if (layers[i].weights.size() == 0)
				{
					std::cerr << "save_network: layer " << i << "'s # of weights is 0\n";
					return;
				}
				neutron::binary::write_block(location, block, layers[i].weights);
				block++;
				if (layers[i].biases.size() == 0)
				{
					std::cerr << "save_network: layer " << i << "'s # of biases is 0\n";
					return;
				}
				neutron::binary::write_block(location, block, layers[i].biases);
				block++;
			}
		}
		// Reads the neural network at a given location and returns it in the `neutron::network` struct.
		static network read_network(char *location)
		{
			neutron::binary::header header = neutron::binary::read_metadata(location);
			neutron::network new_network = network();
			new_network.layers.resize(header.blocks / 2);

			new_network.config_data.resize(header.config_size);
			new_network.config_data = header.config_data;

			uint32_t pointer = 0;

			// Loop through layers
			for (uint32_t layer = 0; layer < (header.blocks / 2); layer++)
			{
				new_network.layers[layer].weights = neutron::binary::read_block(location, pointer);
				pointer++;
				new_network.layers[layer].biases = neutron::binary::read_block(location, pointer);
				pointer++;
			}

			new_network.layers[0].input_size = header.config_data[0];
			new_network.layers[0].output_size = new_network.layers[0].biases.size();

			for (uint32_t layer = 1; layer < new_network.layers.size(); layer++)
			{
				new_network.layers[layer].input_size = new_network.layers[layer - 1].biases.size();

				new_network.layers[layer].output_size = new_network.layers[layer].biases.size();
			}

			return new_network;
		}
		// Passes inputs through a neural network and returns the outputs.
		output forward_pass(std::vector<float> inputs)
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
					next_activations[j] = helper::activation_function(sum);

					// Save activation
					fp_output.activations.push_back(next_activations[j]);
				}

				// These outputs become inputs for the next layer
				fp_output.outputs = std::move(next_activations);
			}

			return fp_output;
		}
		// Compares forward pass output to the target output and calculates how to nudge the neural network closer to the correct output.
		backprop_gradients backpropagation(neutron::network::output output, std::vector<float> target_output)
		{
			// Basic checks
			if (output.activations.empty())
			{
				std::cerr << "backpropagation: activations array is empty\n";
				return neutron::network::backprop_gradients{};
			}
			if (output.pre_activations.empty())
			{
				std::cerr << "backpropagation: pre-activations array is empty\n";
				return neutron::network::backprop_gradients{};
			}
			if (target_output.size() != layers[layers.size()].output_size)
			{
				std::cerr << "backpropagation: target outputs do not match that of the provided neural network\n";
				return neutron::network::backprop_gradients{};
			}
			// Count neurons to check if they match the amount of activations and pre-activations
			int neurons = 0;
			for (int i = 0; i < layers.size(); i++)
				neurons += layers[i].biases.size();
			if (output.activations.size() != neurons)
			{
				std::cerr << "backpropagation: activations array does not match the provided network\n";
				return neutron::network::backprop_gradients{};
			}
			if (output.pre_activations.size() != neurons)
			{
				std::cerr << "backpropagation: pre-activations array does not match the provided network\n";
				return neutron::network::backprop_gradients{};
			}

			backprop_gradients result;

			// Create matrices from vector arrays
			helper::matrix m_output = helper::matrix(1, output.outputs.size(), output.outputs);
			helper::matrix m_target_output = helper::matrix(1, target_output.size(), target_output);
			helper::matrix m_activations = helper::matrix(1, output.activations.size(), output.activations);
			helper::matrix m_pre_activations = helper::matrix(1, output.pre_activations.size(), output.pre_activations);

			// Create temporary buffers to keep gradients in their matrix forms
			std::vector<helper::matrix> bias_error;
			std::vector<helper::matrix> weight_error;

			// Do the output layer seperately
			helper::matrix output_bias_error = (m_output - m_target_output).element(m_pre_activations.map(helper::activation_function_derivative));
			helper::matrix output_weight_error = output_bias_error.dot(m_activations.transpose());

			bias_error.push_back(output_bias_error);
			weight_error.push_back(output_weight_error);

			/*for (uint32_t i = layers.size() - 2, v = 1; i > 0; i -= 1, v++)
			{
				helper::matrix bias_error = helper::matrix(1, layers[i].weights.size(), layers[i].weights).dot(bias_error[v - 1]).element(m_pre_activations.map(helper::activation_function_derivative));
				helper::matrix weight_error = bias_error.dot(m_activations.transpose());
			}*/ // for compilation testing after losing all my shit lol!!
		}
		// Prints out the contents of a neural network.
		void output_network()
		{
			std::cout << "layers: " << layers.size() << "\n\n";
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
				std::cout << "\n    biases:     ";
				for (size_t v = 0; v < layers[i].biases.size(); v++)
				{
					std::cout << layers[i].biases[v] << "  ";
				}
				std::cout << "\n\n";
			}
		}
	};
}

#endif
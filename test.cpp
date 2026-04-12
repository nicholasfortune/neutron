#include "neutron.hpp"

int main() {
    neutron::network my_network = neutron::network({2, 3, 2}); // Create an instance
    my_network.output_network(); // Print the weights and biases
    my_network.save_network("my-network.neu"); // Save the network

    return 0;
}
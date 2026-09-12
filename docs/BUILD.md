# Neutron Building Guide
You can build this project however you want, but for beginners, I recommend g++ from GCC.


# Building the CLI For Windows
To get a CLI, open the root directory in terminal, and then choose from the following commands:


### For a stable CLI build:

`g++ neutroncli.cpp -o bin/neutroncli.exe`


### For an optimized CLI build:

`g++ neutroncli.cpp -o bin/neutroncli.exe -O3 -march=native -mtune=native -flto -DNDEBUG`


# Building the CLI For Linux
To get a CLI, open the root directory in terminal, and then choose from the following commands:


### For a stable CLI build:

`g++ neutroncli.cpp -o bin/neutroncli`


### For an optimized CLI build:

`g++ neutroncli.cpp -o bin/neutroncli -O3 -march=native -mtune=native -flto -DNDEBUG`


# Using Neutron as a library
The entire neutron project can be ran with a single header file, just put it in your project folder and you're done.
```
#include "neutron.hpp"

int main() {
    neutron::network my_network = neutron::network({2, 3, 2}); // Create an instance
    my_network.output_network(); // Print the weights and biases
    my_network.save_network("my-network.neu"); // Save the network

    return 0;
}
```
Simple as that!
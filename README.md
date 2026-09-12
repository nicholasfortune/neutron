**For build instructions, see [BUILD.md](docs/BUILD.md).**

# What is Neutron?
Neutron is a **zero-dependency**, **ultra-lightweight** C++ neural engine designed for **high-performance CPU inference and training** on **any system**.
- **Zero Dependencies:** Built entirely with the C++ Standard Library (`std::`). If you have a compiler, you have a neural network.
- **Ultra-Lightweight, Fully CPU-based:** The whole library is under 1MiB and is natively optimized for CPU execution. Perfect for embedded systems.
- **Dead Simple Custom Binary Format (.neu):** A lean, flat binary structure with a metadata header and alternating raw weights and biases storage. Simple writing, simple reading.
- **Library and CLI:** Works as both a library (**Header file only!**) and a useful CLI for quick neural network creation and training, as well as included developer tools. 

For more info about the `.neu` binary format, see [BINARY.md](docs/BINARY.md)

# License
This software is licensed under the [GNU GPL v3](LICENSE) license. **You are allowed to**:
- Use the software **for any purpose**.
- Copy and share the software **freely**.
- **Modify the software** and run your modified version.
- **Distribute modified or unmodified versions**, including commercially.

You **must**:
- **Provide source code** if you distribute binaries.
- Derivative works **must also be GPLv3**.
- You **cannot add extra restrictions**.
- Installation info **must be provided** if distributing in consumer devices.
- **No warranty** is provided.

The GPL only applies when you distribute code. 
Please read the [whole license for more details.](LICENSE)
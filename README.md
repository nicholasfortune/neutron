## Table of Contents
1. [README.md](README.md) - General info
2. [BUILD.md](docs/BUILD.md) - Build and implimentation info
3. [BINARY.md](docs/BINARY.md) - Custom (`.neu`) file format info

# What is Neutron?
Neutron is a **zero-dependency**, **ultra-lightweight** C++ neural engine designed for **high-performance CPU inference and training** on **any system**.
- **Zero Dependencies:** Built entirely with the C++ Standard Library (`std::`). If you have a compiler, you have a neural network.
- **Ultra-Lightweight, Fully CPU-based:** The whole library (`neutron.hpp`) is under 100KiB and is natively optimized for CPU execution. Perfect for embedded systems.
- **Dead Simple Custom Binary Format (.neu):** A lean, flat binary structure with a metadata header and alternating raw weights and biases storage. Simple writing, simple reading.
- **Library and CLI:** Works as both a library (**Header file only!**) and a useful CLI for quick neural network creation and training, as well as included developer tools. 

For more info about the `.neu` binary format, see [BINARY.md](docs/BINARY.md).

# License
This software is licensed under the [GNU GPL v3](LICENSE) license.
### **Your Rights:**
- **Use:** Run the software for **any purpose**, including commercial and private use.
- **Study & Modify:** Access the source code and modify it to suit your needs.
- **Distribute:** **Copy, share, and distribute** both original and modified versions of the software.

### **Your Obligations:**
- Derivative works **must also be GPLv3**.
- You **cannot add extra restrictions**.
- Installation info **must be provided** if distributing in consumer hardware.

### Disclaimer
**No Warranty:** This software is provided "as is" without warranty of **any kind**.
<br><br><br>
The GPL only applies when you distribute code. 
Please read the [whole license for more details.](LICENSE)
# NTT-based-polynomial-multiplier-FPGA
An FPGA-based coprocessor dedicated to accelerating polynomial multiplication using the Number Theoretic Transform (NTT)


## Overview
This project implements a hardware co-processor designed for efficient polynomial multiplication based on the Number Theoretic Transform (NTT).

## Project Structure
The repository is organized into three main directories:

* **`Hardware_Multiplier/`**: Contains the hardware implementation of the NTT-based multiplier, written in **Verilog HDL**.
* **`NTT_Software/`**: Provides four types of **purely software-based** NTT multipliers. This includes both optimized and unoptimized versions of the **Gentleman-Sande (GS)** and **Cooley-Tukey (CT)** algorithms, intended for **comparison and benchmarking purposes**.
* **`Software_Hardware_Communicator/`**: Implements the **Host-FPGA communication interface** via a custom C program.

> **Note:** Each directory contains its own `README.md` file with detailed instructions on how to compile and execute the respective modules.
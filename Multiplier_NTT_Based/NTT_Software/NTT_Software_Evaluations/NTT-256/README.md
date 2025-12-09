## Overview
This work is based on the research developed by **Tancrède Lepoint (2017)**. It proposes and implements an optimized Number Theoretic Transform (NTT) applying the modular reduction technique described by **Longa and Naehrig (2016)** for the modulus **q = 12289**.

## Project Structure
The software is organized into the following modules:

* **`NTT_Software_Evaluations/`**: Contains the core source code for the NTT/INTT algorithms and the polynomial multiplier.
    * `NTT_RED`: Implementation **with** optimization.
    * `NTT`: Standard implementation **without** optimization.
* **`Generator_Params/`**: Utilities responsible for generating the necessary pre-computed parameters for the NTT/INTT execution.
* **`helpers/`**: Scripts for verification and data conversion.
    * `schoolbook.py`: Verifies results using the standard schoolbook multiplication method.
    * *Converters*: Tools to convert decimal values from `.txt` files to hexadecimal.
    * *Note: These helper scripts were designed to be executed in a **Google Colab** environment.*

## Build and Run
To compile and execute the multiplier program, simply run the Makefile using the following command:

```bash
make
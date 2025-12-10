# Implementation and Execution of the Host-FPGA Interface

This directory contains the implementation of the communication interface via PCIe between the software (Host in C) and the hardware (FPGA).

All necessary files for operation on the **DE2i-150** board are included here. The Host application source code is located in the `linux_app` subdirectory.

## Prerequisites

Before executing the project, verify the following points:

* **PCIe Driver:** Ensure that the board driver is correctly installed and configured.
    * *Reference:* DE2i-150 FPGA System User Manual (Version 1.3, Terasic Inc., 2013).
* **Libraries:** Verify that all necessary libraries have their paths correctly defined in the system environment variables.
* **Memory Mapping (Critical):**
    > **Attention:** Pay close attention to the address parameters defined in the C code. They must strictly correspond to the memory mapping configurations defined in the Qsys (Quartus II) project. Incorrect addresses may cause the system to crash or return invalid data.

## Execution Instructions

Follow the order below to ensure the PCIe communication functions correctly:

1.  **Program the FPGA:**
    Download the bitstream (hardware) to the FPGA board (via JTAG/USB Blaster).

2.  **Reboot:**
    Restart the Host computer.
    > *Note: This step is mandatory for the PCIe bus to correctly enumerate the newly programmed device.*

3.  **Compilation and Execution:**
    Access this directory via terminal and execute the following command:
    ```bash
    make
    ```
    This command will compile the source code and automatically execute the C program, initiating communication with the polynomial multiplication accelerator.
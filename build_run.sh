#!/bin/bash

# Check if an assembly file is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <input_file.asm>"
    exit 1
fi

# Input assembly file
ASM_FILE=$1
OUTPUT_FILE="out.bin"

# Paths to assembler and VM simulator
ASSEMBLER="./deasm"  # Path to the assembler binary
SIMULATOR="./vm"   # Path to the VM simulator binary

# Compile the assembler and simulator if needed
echo "Compiling assembler and simulator..."
gcc -o deasm deasm.c || { echo "Assembler compilation failed"; exit 1; }
gcc -o vm vm.c || { echo "Simulator compilation failed"; exit 1; }

# Assemble the assembly file
echo "Assembling $ASM_FILE into $OUTPUT_FILE..."
$ASSEMBLER "$ASM_FILE" "$OUTPUT_FILE" || { echo "Assembly failed"; exit 1; }

# Run the binary file in the simulator
echo "Running $OUTPUT_FILE in the simulator..."
$SIMULATOR "$OUTPUT_FILE" || { echo "Simulation failed"; exit 1; }

echo "Process complete."

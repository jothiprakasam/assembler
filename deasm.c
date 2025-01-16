#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Define opcodes for instructions
#define OPCODE_MOV 0x1
#define OPCODE_ADD 0x2
#define OPCODE_SUB 0x3
#define OPCODE_MUL 0x4
#define OPCODE_DIV 0x5
#define OPCODE_AND 0x6
#define OPCODE_OR  0x7
#define OPCODE_XOR 0x8
#define OPCODE_CMP 0x9
#define OPCODE_JMP 0xA
#define OPCODE_CALL 0xB
#define OPCODE_RET 0xC
#define OPCODE_SHL 0xD
#define OPCODE_SHR 0xE
#define OPCODE_HLT 0xF

// Get the register code
int get_register_code(const char *reg) {
    if (strcmp(reg, "R0") == 0) return 0;
    if (strcmp(reg, "R1") == 0) return 1;
    if (strcmp(reg, "R2") == 0) return 2;
    if (strcmp(reg, "R3") == 0) return 3;
    return -1; // Invalid register
}

// Assemble a line of code
unsigned short assemble_line(const char *line) {
    char instr[10], arg1[10], arg2[10];
    int opcode = 0, reg_dest = 0, reg_src = 0, immediate = 0;
    unsigned short machine_code = 0;

    // Parse the line
    int parts = sscanf(line, "%s %s %s", instr, arg1, arg2);

    // Remove trailing commas, if any
    char *comma;
    if ((comma = strchr(arg1, ',')) != NULL) *comma = '\0';
    if ((comma = strchr(arg2, ',')) != NULL) *comma = '\0';

    // Determine the opcode
    if (strcmp(instr, "MOV") == 0) opcode = OPCODE_MOV;
    else if (strcmp(instr, "ADD") == 0) opcode = OPCODE_ADD;
    else if (strcmp(instr, "SUB") == 0) opcode = OPCODE_SUB;
    else if (strcmp(instr, "MUL") == 0) opcode = OPCODE_MUL;
    else if (strcmp(instr, "DIV") == 0) opcode = OPCODE_DIV;
    else if (strcmp(instr, "AND") == 0) opcode = OPCODE_AND;
    else if (strcmp(instr, "OR") == 0) opcode = OPCODE_OR;
    else if (strcmp(instr, "XOR") == 0) opcode = OPCODE_XOR;
    else if (strcmp(instr, "CMP") == 0) opcode = OPCODE_CMP;
    else if (strcmp(instr, "JMP") == 0) opcode = OPCODE_JMP;
    else if (strcmp(instr, "CALL") == 0) opcode = OPCODE_CALL;
    else if (strcmp(instr, "RET") == 0) opcode = OPCODE_RET;
    else if (strcmp(instr, "SHL") == 0) opcode = OPCODE_SHL;
    else if (strcmp(instr, "SHR") == 0) opcode = OPCODE_SHR;
    else if (strcmp(instr, "HLT") == 0) opcode = OPCODE_HLT;
    else {
        fprintf(stderr, "Error: Unknown instruction '%s'\n", instr);
        exit(1);
    }

    // Construct the machine code
    machine_code |= (opcode << 12); // Set opcode

    if (opcode == OPCODE_HLT || opcode == OPCODE_RET) {
        return machine_code; // HLT and RET have no operands
    }

    if (opcode == OPCODE_JMP || opcode == OPCODE_CALL) {
        immediate = atoi(arg1); // Immediate value for address
        machine_code |= (immediate & 0xFFF); // 12-bit address
        return machine_code;
    }

    reg_dest = get_register_code(arg1);
    if (reg_dest == -1) {
        fprintf(stderr, "Error: Invalid register '%s'\n", arg1);
        exit(1);
    }
    machine_code |= (reg_dest << 8); // Set destination register

    if (parts == 3) { // Instruction has a second operand
        reg_src = get_register_code(arg2);
        if (reg_src != -1) {
            machine_code |= (reg_src << 4); // Set source register
        } else { // Immediate value
            immediate = atoi(arg2);
            machine_code |= (1 << 7); // Immediate flag
            machine_code |= (immediate & 0xF); // Set immediate value
        }
    }

    return machine_code;
}

// Function to assemble the entire program
void assemble_program(const char *input_file, const char *output_file) {
    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(output_file, "wb");
    char line[100];

    if (!in || !out) {
        fprintf(stderr, "Error: Unable to open files\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), in)) {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == ';') continue;

        // Assemble the line
        unsigned short machine_code = assemble_line(line);
        fwrite(&machine_code, sizeof(machine_code), 1, out);
    }

    fclose(in);
    fclose(out);
    printf("Assembly complete. Output written to %s\n", output_file);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file.asm> <output_file.bin>\n", argv[0]);
        return 1;
    }

    assemble_program(argv[1], argv[2]);
    return 0;
}

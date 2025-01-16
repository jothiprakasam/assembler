#include <stdio.h>
#include <stdlib.h>

#define OPCODE_MOV  0x1
#define OPCODE_ADD  0x2
#define OPCODE_SUB  0x3
#define OPCODE_MUL  0x4
#define OPCODE_DIV  0x5
#define OPCODE_CMP  0x9
#define OPCODE_JMP  0xA
#define OPCODE_CALL 0xB
#define OPCODE_RET  0xC
#define OPCODE_SHL  0xD
#define OPCODE_SHR  0xE
#define OPCODE_HLT  0xF

#define STACK_SIZE 256

// Virtual Machine CPU structure
typedef struct {
    int registers[4];
    int pc; // Program Counter
    int sp; // Stack Pointer
    int halted;
    unsigned short stack[STACK_SIZE];
} CPU;

// Fetch an instruction from the binary file
unsigned short fetch_instruction(FILE *bin_file, int *pc) {
    unsigned short instruction;
    fseek(bin_file, (*pc) * sizeof(instruction), SEEK_SET);
    if (fread(&instruction, sizeof(instruction), 1, bin_file) == 1) {
        (*pc)++;
        return instruction;
    }
    return 0xFFFF; // End of file or error
}

// Execute a single instruction
void execute_instruction(CPU *cpu, unsigned short instruction, FILE *bin_file) {
    int opcode = (instruction >> 12) & 0xF;
    int reg_dest = (instruction >> 8) & 0xF;
    int immediate_flag = (instruction >> 7) & 0x1;
    int reg_src = (instruction >> 4) & 0x7;
    int immediate_value = instruction & 0x7F; // Use 7 bits for immediate value

    switch (opcode) {
        case OPCODE_MOV:
            if (immediate_flag) {
                cpu->registers[reg_dest] = immediate_value;
            } else {
                cpu->registers[reg_dest] = cpu->registers[reg_src];
            }
            break;

        case OPCODE_ADD:
            cpu->registers[reg_dest] += cpu->registers[reg_src];
            break;

        case OPCODE_SUB:
            cpu->registers[reg_dest] -= cpu->registers[reg_src];
            break;

        case OPCODE_MUL:
            cpu->registers[reg_dest] *= cpu->registers[reg_src];
            break;

        case OPCODE_DIV:
            if (cpu->registers[reg_src] != 0) {
                cpu->registers[reg_dest] /= cpu->registers[reg_src];
            } else {
                fprintf(stderr, "Error: Division by zero\n");
                cpu->halted = 1;
            }
            break;

        case OPCODE_CMP:
            cpu->registers[reg_dest] = (cpu->registers[reg_dest] == cpu->registers[reg_src]) ? 0 : 1;
            break;

        case OPCODE_JMP:
            cpu->pc = immediate_value;
            break;

        case OPCODE_CALL:
            if (cpu->sp < STACK_SIZE) {
                cpu->stack[cpu->sp++] = cpu->pc;
                cpu->pc = immediate_value;
            } else {
                fprintf(stderr, "Error: Stack overflow\n");
                cpu->halted = 1;
            }
            break;

        case OPCODE_RET:
            if (cpu->sp > 0) {
                cpu->pc = cpu->stack[--cpu->sp];
            } else {
                fprintf(stderr, "Error: Stack underflow\n");
                cpu->halted = 1;
            }
            break;

        case OPCODE_SHL:
            cpu->registers[reg_dest] <<= immediate_value;
            break;

        case OPCODE_SHR:
            cpu->registers[reg_dest] >>= immediate_value;
            break;

        case OPCODE_HLT:
            cpu->halted = 1;
            break;

        default:
            fprintf(stderr, "Unknown opcode: %d\n", opcode);
            cpu->halted = 1;
    }
}

// Run the program from the binary file
void run_program(const char *bin_file_path) {
    FILE *bin_file = fopen(bin_file_path, "rb");
    if (!bin_file) {
        fprintf(stderr, "Error: Could not open file %s\n", bin_file_path);
        exit(1);
    }

    CPU cpu = {{0, 0, 0, 0}, 0, 0, 0, {0}};

    printf("Starting simulation...\n");
    while (!cpu.halted) {
        unsigned short instruction = fetch_instruction(bin_file, &cpu.pc);
        if (instruction == 0xFFFF) {
            printf("End of file reached.\n");
            break;
        }
        execute_instruction(&cpu, instruction, bin_file);
    }

    fclose(bin_file);

    // Print register values after execution
    printf("Simulation complete. Final register values:\n");
    for (int i = 0; i < 4; i++) {
        printf("R%d: %d\n", i, cpu.registers[i]);
    }
    printf("Final Stack Pointer: %d\n", cpu.sp);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.bin>\n", argv[0]);
        return 1;
    }
    run_program(argv[1]);
    return 0;
}

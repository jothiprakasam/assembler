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
#define MEMORY_SIZE 1024

typedef struct {
    int registers[4];
    int pc; // program counter
    int sp; // stack pointer
    int bp; // base pointer
    int halted;
    unsigned short memory[MEMORY_SIZE]; // Virtual memory
    unsigned short stack[STACK_SIZE];  // Stack memory
} CPU;

// Fetch instruction from memory
unsigned short fetch_instruction(CPU *cpu) {
    if (cpu->pc < 0 || cpu->pc >= MEMORY_SIZE - 1) {
        fprintf(stderr, "Error: PC out of bounds\n");
        cpu->halted = 1;
        return 0xFFFF;
    }

    // Fetch two bytes from memory and form a 16-bit instruction
    unsigned char low_byte = cpu->memory[cpu->pc];      // Lower 8 bits
    unsigned char high_byte = cpu->memory[cpu->pc + 1]; // Higher 8 bits

    // Increment PC by 2 since instructions are 16-bit
    cpu->pc += 2;

    // Combine bytes (assuming little-endian storage)
    return (high_byte << 8) | low_byte;
}

// Read from memory
unsigned short read_memory(CPU *cpu, int address) {
    if (address < 0 || address >= MEMORY_SIZE) {
        fprintf(stderr, "Memory Error: Address out of bounds\n");
        exit(1);
    }
    return cpu->memory[address];
}

// Write to memory
void write_memory(CPU *cpu, int address, unsigned short value) {
    if (address < 0 || address >= MEMORY_SIZE) {
        fprintf(stderr, "Memory Error: Address out of bounds\n");
        exit(1);
    }
    cpu->memory[address] = value;
}

// Execute a single instruction
void execute_instruction(CPU *cpu, unsigned short instruction) {
    int opcode = (instruction >> 12) & 0xF;
    int reg_dest = (instruction >> 8) & 0xF;
    int memory_flag = (instruction >> 7) & 0x1;
    int reg_src = (instruction >> 4) & 0x7;
    int immediate_or_addr = instruction & 0x7F; // Address or immediate value

    switch (opcode) {
        case OPCODE_MOV:
            if (memory_flag) {
                if (reg_src == 0x7) { // Load from memory
                    cpu->registers[reg_dest] = read_memory(cpu, immediate_or_addr);
                } else { // Store to memory
                    write_memory(cpu, immediate_or_addr, cpu->registers[reg_dest]);
                }
            } else if (reg_src == 0x7) { // Immediate value
                cpu->registers[reg_dest] = immediate_or_addr;
            } else { // Register-to-register
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
            cpu->pc = immediate_or_addr;
            break;

        case OPCODE_CALL:
            if (cpu->sp < STACK_SIZE) {
                cpu->stack[cpu->sp++] = cpu->pc;
                cpu->pc = immediate_or_addr;
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
            cpu->registers[reg_dest] <<= immediate_or_addr;
            break;

        case OPCODE_SHR:
            cpu->registers[reg_dest] >>= immediate_or_addr;
            break;

        case OPCODE_HLT:
            cpu->halted = 1;
            break;

        default:
            fprintf(stderr, "Unknown opcode: %d\n", opcode);
            cpu->halted = 1;
    }
}

// Run the program from memory
void run_program(const char *bin_file_path) {
    FILE *bin_file = fopen(bin_file_path, "rb");
    if (!bin_file) {
        fprintf(stderr, "Error: Could not open file %s\n", bin_file_path);
        exit(1);
    }

    CPU cpu = {{0}, 0, 0, 0, 0, {0}, {0}};
    fread(cpu.memory, sizeof(unsigned short), MEMORY_SIZE, bin_file);
    fclose(bin_file);

    printf("Starting simulation...\n");
    while (!cpu.halted) {
        unsigned short instruction = fetch_instruction(&cpu);
        execute_instruction(&cpu, instruction);
    }

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

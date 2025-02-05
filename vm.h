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

typedef struct {
    int registers[4];
    int pc;
    int sp;
    int halted;
    unsigned short stack[STACK_SIZE];
}CPU;
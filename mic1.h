#include <stdint.h>
#include <stdio.h>

typedef enum {
  RETURN_A = 0x18,
  RETURN_B = 0x14,
  INVERSE_A = 0x1A,
  INVERSE_B = 0x2C,
  A_PLUS_B = 0x3C,
  A_PLUS_B_PLUS_1 = 0x3D,
  A_PLUS_1 = 0x39,
  B_PLUS_1 = 0x37,
  B_MINUS_A = 0x3F,
  B_MINUS_1 = 0x36,
  MINUS_A = 0x3B,
  A_AND_B = 0x0B,
  A_OR_B = 0x1B,
  ZERO = 0x10,
  ONE = 0x31,
  MINUS_ONE = 0x32,
  SHIFT_LEFT = 0x01,
  SHIFT_RIGHT = 0x02
} ALU_OP_t;

typedef enum {
  REG_MDR,
  REG_PC,
  REG_MBR,
  REG_MBRU,
  REG_SP,
  REG_LV,
  REG_CPP,
  REG_TOS,
  REG_OPC,
} REG_ADDR_t;

// MIR: micro instruction registers
typedef struct {
  uint16_t ADDR : 9;
  uint8_t JAMN  : 1;
  uint8_t JAMZ  : 1;
  uint8_t JMPC  : 1;
  uint8_t ALU   : 8;
  uint16_t C    : 9;
  uint8_t M     : 3;
  uint8_t B     : 4;
} MIR_t;

typedef struct MIC1_s {
  int8_t program[2048];
  int32_t data[2048];
  int32_t stack[2048];
  int32_t constant[2048];
  MIR_t control_store[512];

  uint16_t MPC : 9; // microprogram counter
  uint8_t Z : 1; // ALU calculated zero
  uint8_t N : 1; // ALU calculated negative

  uint32_t MAR; // memory address register
  int32_t MDR; // memory data register

  union {
    int32_t MBR; // memory buffer register
    uint32_t MBRU; // unsigned memory buffer register
  };

  uint32_t PC; // program counter
  uint32_t OPC; // old program counter

  uint32_t SP; // stack pointer
  uint32_t LV; // local variable (aka frame pointer)
  uint32_t TOS; // top of stack

  uint32_t CPP; // constant pool pointer

  int32_t H; // H register, which is always the ALU A-value
} MIC1_t;

//update N and Z flag based on if value is negative or zero
void updateNZ(MIC1_t *mic, int32_t value);

int32_t alu(MIC1_t *mic);
int32_t shifter(MIC1_t *mic, int32_t value);

//read what's on the B-bus
int32_t bbus(MIC1_t *mic);
//write C-bus value to specified registers
void cbus(MIC1_t *mic, int32_t value);

//update the MPC based on the executed instruction
void addr(MIC1_t *mic);

void fetch(MIC1_t *mic);
void write(MIC1_t *mic);
void read(MIC1_t *mic);

void mic1_interp(MIC1_t *mic);

#include <stdint.h>
#include <stdio.h>

enum {
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
  MINUS_ONE = 0x32
};

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

struct control_store_s {
  uint16_t ADDR : 9;
  uint8_t JAMN  : 1;
  uint8_t JAMZ  : 1;
  uint8_t JMPC  : 1;
  uint8_t ALU   : 8;
  uint16_t C    : 9;
  uint8_t M     : 3;
  uint8_t B     : 4;
};

typedef union {
  struct control_store_s bits;
  uint64_t full : 36;
} control_store_t;

typedef struct MIC1_s {
  int32_t program[2048];
  int32_t data[2048];
  int32_t stack[2048];
  int32_t constant[2048];
  control_store_t control_store[512];

  uint16_t MPC : 9;
  uint8_t Z : 1;
  uint8_t N : 1;

  uint32_t MAR;
  int32_t MDR;
  union { int8_t MBR; uint8_t MBRU; };
  uint32_t PC, OPC, SP, LV, CPP;
  int32_t H, TOS;
} MIC1_t;

int32_t alu(MIC1_t *mic);
void updateNZ(MIC1_t *mic, int32_t value);
int32_t bbus(MIC1_t *mic);
int32_t shifter(MIC1_t *mic, int32_t value);
void addr(MIC1_t *mic);
void cbus(MIC1_t *mic);

void mic1_interp(MIC1_t *mic);

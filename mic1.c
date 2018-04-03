#include "mic1.h"

int32_t bbus(MIC1_t *mic) {
  switch (mic->control_store[mic->MPC].B) {
    case REG_MDR:
      return mic->MDR;
    case REG_PC:
      return mic->PC;
    case REG_MBR:
      return mic->MBR;
    case REG_MBRU:
      return mic->MBRU;
    case REG_SP:
      return mic->SP;
    case REG_LV:
      return mic->LV;
    case REG_CPP:
      return mic->CPP;
    case REG_TOS:
      return mic->TOS;
    case REG_OPC:
      return mic->OPC;
  }
}

void cbus(MIC1_t *mic, int32_t value) {
  uint8_t C = mic->control_store[mic->MPC].C;

  if (C & 0x001) {
    mic->MAR = (uint32_t)value;
  } else if (C & 0x002) {
    mic->MDR = value;
  } else if (C & 0x004) {
    mic->PC = (uint32_t)value;
  } else if (C & 0x008) {
    mic->SP = (uint32_t)value;
  } else if (C & 0x010) {
    mic->LV = (uint32_t)value;
  } else if (C & 0x020) {
    mic->CPP = (uint32_t)value;
  } else if (C & 0x040) {
    mic->TOS = value;
  } else if (C & 0x080) {
    mic->OPC = (uint32_t)value;
  } else if (C & 0x100) {
    mic->H = value;
  }
}

void updateNZ(MIC1_t *mic, int32_t value) {
  if (value == 0) {
    mic->N = 0;
    mic->Z = 1;
  } else if (value < 0) {
    mic->Z = 0;
    mic->N = 1;
  } else {
    mic->Z = 0;
    mic->N = 0;
  }
}

int32_t alu(MIC1_t *mic) {
  uint8_t alu_op = mic->control_store[mic->MPC].ALU;
  int32_t tmp;

  // 0x3F masks all the non-ALU bits in ALU (masks shift control bits)
  switch (alu_op & 0x3F) {
    case RETURN_A:
      tmp = mic->H;

      break;
    case RETURN_B:
      tmp = bbus(mic);
      updateNZ(mic, tmp);
      return tmp;
    case INVERSE_A:
      tmp = ~(mic->H);
      break;
    case INVERSE_B:
      tmp = bbus(mic);
      break;
    case A_PLUS_B:
      tmp = mic->H + bbus(mic);
      break;
    case A_PLUS_B_PLUS_1:
      tmp = mic->H + bbus(mic) + 1;
      break;
    case A_PLUS_1:
      tmp = mic->H + 1;
      break;
    case B_PLUS_1:
      tmp = bbus(mic) + 1;
      break;
    case B_MINUS_A:
      tmp = bbus(mic) - mic->H;
      break;
    case B_MINUS_1:
      tmp = bbus(mic) - 1;
      break;
    case MINUS_A:
      tmp = -(mic->H);
      break;
    case A_AND_B:
      tmp = mic->H & bbus(mic);
      break;
    case A_OR_B:
      tmp = mic->H | bbus(mic);
      break;
    case ZERO:
      tmp = 0;
      break;
    case ONE:
      tmp = 1;
      break;
    case MINUS_ONE:
      tmp = -1;
      break;
    default:
      tmp = 0;
  }

  updateNZ(mic, tmp);
  return tmp;
}

int32_t shifter(MIC1_t *mic, int32_t value) {
  uint8_t shift_op = mic->control_store[mic->MPC].ALU;
  int32_t tmp;


  // 0xC0 masks all the non-SHIFT bits in ALU (masks ALU control bits)
  // afterwards this is shifted 6 to the right to remove the non-SHIFT bits
  switch ((shift_op & 0xC0) >> 6) {
    case SHIFT_RIGHT:
      return value >> 8;
    case SHIFT_LEFT:
      return value << 8;
    default:
      return value;
  }
}

void addr(MIC1_t *mic) {
  uint8_t JMPC = mic->control_store[mic->MPC].JMPC;
  uint8_t JAMZ = mic->control_store[mic->MPC].JAMZ;
  uint8_t JAMN = mic->control_store[mic->MPC].JAMN;
  uint8_t MBR = mic->MBR;
  uint16_t ADDR = mic->control_store[mic->MPC].ADDR;

  if (JMPC) {
    mic->MPC = MBR | ADDR;
  } else {
    mic->MPC = ADDR | (((JAMZ & mic->Z) | (JAMN & mic->N)) << 8);
  }
}

void read(MIC1_t *mic) {
    mic->MDR = mic->data[mic->MAR];
}

void write(MIC1_t *mic) {
    mic->data[mic->MAR] = mic->MDR;
}

void fetch(MIC1_t *mic) {
    mic->MBR = mic->program[mic->PC];
}

void mic1_interp(MIC1_t *mic) {
  MIR_t MIR;
  int32_t value;

  while (1) {
    MIR = mic->control_store[mic->MPC];
    value = shifter(mic, alu(mic));

    if (MIR.M == 1) {
        fetch(mic);
    } else if (MIR.M == 2) {
        write(mic);
    } else if (MIR.M == 4) {
        read(mic);
    }

    addr(mic);
  }
}

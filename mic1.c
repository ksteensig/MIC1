#include "mic1.h"

int32_t bbus(MIC1_t *mic) {
  switch (mic->control_store[mic->MPC].bits.B) {
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
  uint8_t C = mic->control_store[mic->MPC].bits.C;

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
  uint8_t alu_op = mic->control_store[mic->MPC].bits.ALU;
  int32_t tmp;

  switch (alu_op & 0x3F) {
    case RETURN_A:
      updateNZ(mic, mic->H);
      return mic->H;
    case RETURN_B:
      tmp = bbus(mic);
      updateNZ(mic, tmp);
      return tmp;
    case INVERSE_A:
      updateNZ(mic, ~mic->H);
      return ~mic->H;
    case INVERSE_B:
      tmp = bbus(mic);
      updateNZ(mic, ~tmp);
      return ~tmp;
    case A_PLUS_B:
      tmp = mic->H + bbus(mic);
      updateNZ(mic, tmp);
      return tmp;
    case A_PLUS_B_PLUS_1:
      tmp = mic->H + bbus(mic) + 1;
      updateNZ(mic, tmp);
      return tmp;
    case A_PLUS_1:
      tmp = mic->H + 1;
      updateNZ(mic, tmp);
      return tmp;
    case B_PLUS_1:
      tmp = bbus(mic) + 1;
      updateNZ(mic, tmp);
      return tmp;
    case B_MINUS_A:
      tmp = bbus(mic) - mic->H;
      updateNZ(mic, tmp);
      return tmp;
    case B_MINUS_1:
      tmp = bbus(mic) - 1;
      updateNZ(mic, tmp);
      return tmp;
    case MINUS_A:
      updateNZ(mic, -(mic->H));
      return -(mic->H);
    case A_AND_B:
      tmp = mic->H & bbus(mic);
      updateNZ(mic, tmp);
      return tmp;
    case A_OR_B:
      tmp = mic->H | bbus(mic);
      updateNZ(mic, tmp);
      return tmp;
    case ZERO:
      updateNZ(mic, 0);
      return 0;
    case ONE:
      updateNZ(mic, 1);
      return 1;
    case MINUS_ONE:
      updateNZ(mic, -1);
      return -1;
    default:
      return 0;
  }
}

int32_t shifter(MIC1_t *mic, int32_t value) {
  uint8_t shift_op = mic->control_store[mic->MPC].bits.ALU;
  int32_t tmp;

  switch ((shift_op & 0xC0) >> 6) {
    case 0x01:
      return value >> 8;
    case 0x02:
      return value << 8;
    default:
      return value;
  }
}

void addr(MIC1_t *mic) {
  uint8_t JMPC = mic->control_store[mic->MPC].bits.JMPC;
  uint8_t JAMZ = mic->control_store[mic->MPC].bits.JAMZ;
  uint8_t JAMN = mic->control_store[mic->MPC].bits.JAMN;
  uint8_t MBR = mic->MBR;
  uint16_t ADDR = mic->control_store[mic->MPC].bits.ADDR;

  if (JMPC) {
    mic->MPC = MBR | ADDR;
  } else {
    mic->MPC = ADDR | ((JAMZ | JAMN) << 8);
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
  struct control_store_s MIR;
  int32_t value;

  while (1) {
    MIR = mic->control_store[mic->MPC].bits;
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

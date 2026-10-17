// callers: 0x255450 0x25d330 0x25d450

void FUN_00250ac0(int param_1,int param_2,uint param_3)

{
  if (param_2 < 0xc) {
    DAT_00260d98 = DAT_00260d98 | 1 << ((byte)param_1 & 0x1f);
    (&DAT_00260da0)[param_1 * 0x20 + param_2] = param_3;
  }
  else {
    if (param_2 == 0xc) {
      DAT_00260d98 = DAT_00260d98 | (-(uint)(param_3 < 0x19) & 0xffffbff1) + 0x480f;
      (&DAT_00260dd0)[param_1 * 0x20] = param_3;
      return;
    }
    if (param_2 < 0x16) {
      DAT_00260d98 = DAT_00260d98 | *(uint *)(&DAT_00278538 + param_2 * 4);
      (&DAT_00260da0)[param_1 * 0x20 + param_2] = param_3;
      return;
    }
    if (param_2 == 0x1c) {
      FUN_002508c0(param_1,param_3);
      return;
    }
    if (param_2 == 0x1d) {
      FUN_00250a40(param_1,param_3);
      return;
    }
    if (param_2 == 0x1e) {
      FUN_00250a80(param_1,param_3);
      return;
    }
    if (param_2 < 0x1c) {
      FUN_002509d0();
      return;
    }
  }
  return;
}


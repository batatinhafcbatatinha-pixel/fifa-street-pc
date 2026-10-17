// callers: 0x251550 0x251570

void FUN_00250e70(void)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar3 = DAT_00261238;
  uVar1 = DAT_00261238[0x90a];
  DAT_00261238[0x90a] = uVar1 & 0xfffffff7;
  if (DAT_00261234 != 0) {
    puVar3[0x90a] = uVar1 & 0xfffffff7 | 8;
  }
  uVar1 = puVar3[0x90b];
  puVar3[0x90b] = uVar1 & 0xe7efffff;
  if (DAT_0026122c != 0) {
    puVar3[0x90b] = uVar1 & 0xe7efffff | 0x100000;
  }
  if (DAT_00261230 != 0) {
    puVar3[0x90b] = puVar3[0x90b] | 0x8000000;
  }
  puVar4 = (undefined4 *)*puVar3;
  if ((undefined4 *)puVar3[1] <= puVar4) {
    puVar4 = (undefined4 *)FUN_00252040();
  }
  *puVar4 = 0x40110;
  puVar4[1] = 0;
  uVar2 = puVar3[0x90a];
  puVar4[2] = &DAT_00081d8c;
  puVar4[3] = 0x400094;
  puVar4[4] = uVar2;
  puVar4[5] = 0x40100;
  puVar4[6] = 9;
  uVar2 = puVar3[0x90b];
  puVar4[7] = &DAT_00081d8c;
  puVar4[8] = 0x400b80;
  puVar4[9] = uVar2;
  puVar4[10] = 0x40100;
  puVar4[0xb] = 9;
  *puVar3 = puVar4 + 0xc;
  return;
}


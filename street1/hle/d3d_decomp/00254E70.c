// callers: 0x198750 0x255250

void FUN_00254e70(uint param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  puVar1 = DAT_00261238;
  if ((param_1 & 0x10) == 0) {
    uVar3 = DAT_00261238[2] & 0xfffffdff;
  }
  else {
    uVar3 = DAT_00261238[2] | 0x200;
  }
  DAT_00261238[2] = uVar3;
  puVar1[0x64a] = param_1 & 0xffffffef;
  if ((param_1 & 0xffffffef) == 0) {
    DAT_00260d98 = DAT_00260d98 | 0x1600;
    puVar2 = (undefined4 *)*puVar1;
    if ((undefined4 *)puVar1[1] <= puVar2) {
      puVar2 = (undefined4 *)FUN_00252040();
    }
    *puVar2 = 0x41ea4;
    puVar2[1] = 0x3c;
    puVar2[2] = &DAT_00300b80;
    puVar5 = &DAT_0025f094;
    puVar6 = puVar2 + 3;
    for (iVar4 = 0xc; iVar4 != 0; iVar4 = iVar4 + -1) {
      *puVar6 = *puVar5;
      puVar5 = puVar5 + 1;
      puVar6 = puVar6 + 1;
    }
    FUN_00251780(puVar2 + 0xf,0x400840,&DAT_0025eef8);
    FUN_00251780(puVar2 + 0x20,0x400880,&DAT_0025eef8);
    FUN_00251780(puVar2 + 0x31,0x4008c0,&DAT_0025eef8);
    FUN_00251780(puVar2 + 0x42,0x400900,&DAT_0025eef8);
    puVar2[0x53] = FUN_001009d0;
    puVar2[0x54] = 0;
    puVar2[0x55] = 0;
    puVar2[0x57] = 0;
    puVar2[0x56] = 0x3f800000;
    puVar2[0x58] = 0x100a50;
    puVar2[0x59] = 0;
    puVar2[0x5a] = 0;
    puVar2[0x5b] = 0;
    puVar2[0x5c] = 0x3f800000;
    *puVar1 = puVar2 + 0x5d;
  }
  return;
}


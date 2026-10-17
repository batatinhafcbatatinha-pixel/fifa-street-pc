// callers: 0x1a0840

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0025e0c0(int param_1,undefined1 (*param_2) [16],int param_3)

{
  int iVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined4 *puVar4;
  uint uVar5;
  undefined1 auVar6 [16];
  
  puVar2 = DAT_00261238;
  iVar1 = *(int *)(DAT_00261238[0x1e1] + 8);
  for (; param_3 != 0; param_3 = param_3 + -1) {
    auVar6 = minps(*param_2,_DAT_002dbd50);
    auVar6 = maxps(auVar6,_DAT_00307090);
    uVar3 = (int)ROUND(auVar6._0_4_ * _DAT_002dbd60) << 0x10 |
            (int)ROUND(auVar6._4_4_ * fRam002dbd64) << 8 | (int)ROUND(auVar6._8_4_ * fRam002dbd68) |
            (int)ROUND(auVar6._12_4_ * fRam002dbd6c) << 0x18;
    puVar2[param_1 + 0x239] = uVar3;
    puVar4 = (undefined4 *)*puVar2;
    if ((undefined4 *)puVar2[1] <= puVar4) {
      puVar4 = (undefined4 *)FUN_00252040();
    }
    uVar5 = *(uint *)(&DAT_002dbd70 + param_1 * 4) ^ *(uint *)(iVar1 + 0xe4);
    if ((uVar5 & 0xf) == 0) {
      _DAT_00260fc8 = uVar3;
      *puVar4 = 0x40a60;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf0) == 0) {
      _DAT_00260fcc = uVar3;
      *puVar4 = 0x40a64;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf00) == 0) {
      _DAT_00260fd0 = uVar3;
      *puVar4 = 0x40a68;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf000) == 0) {
      _DAT_00260fd4 = uVar3;
      *puVar4 = 0x40a6c;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf0000) == 0) {
      _DAT_00260fd8 = uVar3;
      *puVar4 = 0x40a70;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf00000) == 0) {
      _DAT_00260fdc = uVar3;
      *puVar4 = 0x40a74;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf000000) == 0) {
      _DAT_00260fe0 = uVar3;
      *puVar4 = 0x40a78;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf0000000) == 0) {
      _DAT_00260fe4 = uVar3;
      *puVar4 = 0x40a7c;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    uVar5 = *(uint *)(&DAT_002dbd70 + param_1 * 4) ^ *(uint *)(iVar1 + 0xe8);
    if ((uVar5 & 0xf) == 0) {
      _DAT_00260fe8 = uVar3;
      *puVar4 = 0x40a80;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf0) == 0) {
      _DAT_00260fec = uVar3;
      *puVar4 = 0x40a84;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf00) == 0) {
      _DAT_00260ff0 = uVar3;
      *puVar4 = 0x40a88;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf000) == 0) {
      _DAT_00260ff4 = uVar3;
      *puVar4 = 0x40a8c;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf0000) == 0) {
      _DAT_00260ff8 = uVar3;
      *puVar4 = 0x40a90;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf00000) == 0) {
      _DAT_00260ffc = uVar3;
      *puVar4 = 0x40a94;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf000000) == 0) {
      _DAT_00261000 = uVar3;
      *puVar4 = 0x40a98;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf0000000) == 0) {
      _DAT_00261004 = uVar3;
      *puVar4 = 0x40a9c;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    uVar5 = *(uint *)(&DAT_002dbd70 + param_1 * 4) ^ *(uint *)(iVar1 + 0xec);
    if ((uVar5 & 0xf) == 0) {
      _DAT_0026104c = uVar3;
      *puVar4 = 0x41e20;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if ((uVar5 & 0xf0) == 0) {
      _DAT_00261050 = uVar3;
      *puVar4 = 0x41e24;
      puVar4[1] = uVar3;
      puVar4 = puVar4 + 2;
    }
    if (param_1 == 0) {
      *puVar4 = 0xc181c;
      puVar4[1] = *(undefined4 *)*param_2;
      puVar4[2] = *(undefined4 *)(*param_2 + 4);
      puVar4[3] = *(undefined4 *)(*param_2 + 8);
      puVar4 = puVar4 + 4;
    }
    *puVar2 = puVar4;
    param_2 = param_2 + 1;
    param_1 = param_1 + 1;
  }
  return;
}


// callers: 0x19f360 0x1a06e0 0x25d330 0x25d450

void FUN_0025dec0(uint param_1)

{
  uint uVar1;
  undefined4 uVar2;
  uint *puVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  uint uVar6;
  int iVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  
  puVar3 = DAT_00261238;
  uVar1 = DAT_00261238[0x1e1];
  DAT_00261238[0x1e1] = param_1;
  if (param_1 == 0) {
    uVar1 = DAT_00260d98 | 0x4800;
    if (puVar3[0x1e2] != 0) {
      uVar1 = DAT_00260d98 | 0x6800;
    }
    DAT_00260d98 = uVar1;
    FUN_00250510(DAT_002611f0);
    if (puVar3[1] <= *puVar3) {
      FUN_00252040();
    }
    puVar4 = (undefined4 *)FUN_00250bb0();
    *puVar4 = 0x41e78;
    puVar4[1] = 0x210000;
    *puVar3 = (uint)(puVar4 + 2);
    return;
  }
  uVar6 = *(uint *)(*(int *)(param_1 + 8) + 0x24) | *(uint *)(*(int *)(param_1 + 8) + 0x20);
  if ((uVar6 == 0) && (puVar3[0x1e2] != 0)) {
    DAT_00260d98 = DAT_00260d98 | 0x2000;
  }
  puVar3[0x1e2] = uVar6;
  puVar3[0x1e3] = *(uint *)(*(int *)(param_1 + 8) + 0xec) & 0x100;
  puVar4 = *(undefined4 **)(param_1 + 8);
  puVar3[0x1e4] = puVar4[0x36];
  DAT_00260d98 = DAT_00260d98 | 0x4000;
  puVar5 = (undefined4 *)*puVar3;
  if ((undefined4 *)puVar3[1] <= puVar5) {
    puVar5 = (undefined4 *)FUN_00252040();
  }
  if ((puVar3[2] & 0x10) == 0) {
    puVar9 = puVar4;
    puVar8 = &DAT_00260fa0;
    for (iVar7 = 0x39; iVar7 != 0; iVar7 = iVar7 + -1) {
      *puVar8 = *puVar9;
      puVar9 = puVar9 + 1;
      puVar8 = puVar8 + 1;
    }
    DAT_002611c0 = puVar4[0x36];
  }
  if (uVar1 == 0) {
    puVar5 = (undefined4 *)FUN_00250bb0();
  }
  *puVar5 = &PTR_LAB_00200260;
  puVar8 = puVar4;
  puVar9 = puVar5;
  for (iVar7 = 8; puVar9 = puVar9 + 1, iVar7 != 0; iVar7 = iVar7 + -1) {
    *puVar9 = *puVar8;
    puVar8 = puVar8 + 1;
  }
  puVar5[9] = 0x800a60;
  puVar9 = puVar4 + 10;
  puVar8 = puVar5 + 10;
  for (iVar7 = 0x20; iVar7 != 0; iVar7 = iVar7 + -1) {
    *puVar8 = *puVar9;
    puVar9 = puVar9 + 1;
    puVar8 = puVar8 + 1;
  }
  uVar2 = puVar4[0x2a];
  puVar5[0x2a] = 0x417f8;
  puVar5[0x2b] = uVar2;
  puVar5[0x2c] = 0x81e20;
  puVar5[0x2d] = puVar4[0x2b];
  puVar5[0x2e] = puVar4[0x2c];
  puVar5[0x2f] = 0x241e40;
  puVar9 = puVar4 + 0x2d;
  puVar8 = puVar5 + 0x30;
  for (iVar7 = 9; iVar7 != 0; iVar7 = iVar7 + -1) {
    *puVar8 = *puVar9;
    puVar9 = puVar9 + 1;
    puVar8 = puVar8 + 1;
  }
  puVar5[0x39] = 0x81e74;
  puVar5[0x3a] = puVar4[0x37];
  puVar5[0x3b] = puVar4[0x38];
  if (puVar3[0x1e2] != 0) {
    puVar5[0x3c] = &PTR_s_cSlideTackle_00080288;
    puVar5[0x3d] = puVar4[8];
    puVar5[0x3e] = puVar4[9];
    *puVar3 = (uint)(puVar5 + 0x3f);
    return;
  }
  *puVar3 = (uint)(puVar5 + 0x3c);
  return;
}


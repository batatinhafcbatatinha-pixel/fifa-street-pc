// callers: 0x259c6a

void FUN_0025b99e(uint param_1,uint param_2,uint param_3,int param_4)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  int *in_ECX;
  int iVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  
  iVar2 = *in_ECX;
  uVar3 = in_ECX[0x40];
  *(undefined4 *)(iVar2 + 0x2500) = 0;
  uVar5 = 0;
  do {
    iVar4 = uVar5 + 0x1c0000;
    uVar5 = uVar5 + 1;
    *(undefined4 *)(iVar2 + (iVar4 + in_ECX[0x43] * 4) * 4) = 0;
  } while (uVar5 < 0xdfc);
  *(int *)(iVar2 + (uVar3 + 0x1c0000 + in_ECX[0x50] * 4) * 4) = in_ECX[0x43];
  in_ECX[uVar3 + 0x4e] = in_ECX[0x43];
  puVar6 = (undefined4 *)(uVar3 * 0x40 + in_ECX[0x4a] + iVar2);
  iVar4 = 0x10;
  puVar7 = puVar6;
  do {
    *puVar7 = 0;
    puVar7 = puVar7 + 1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  puVar6[3] = *(undefined4 *)(param_4 + 0xc);
  if (param_1 < 8) {
    param_1 = 8;
  }
  if (0x100 < param_1) {
    param_1 = 0;
  }
  if (param_2 < 0x20) {
    param_2 = 0x20;
  }
  if (0x100 < param_2) {
    param_2 = 0;
  }
  if (0xf < param_3) {
    param_3 = 0xf;
  }
  iVar4 = (((param_3 & 0x1f) << 3 | (param_2 >> 5) - 1 & 7) << 10 | (param_1 >> 3) - 1 & 0x1f) << 3;
  puVar6[5] = iVar4;
  uVar5 = 1 << ((byte)uVar3 & 0x1f);
  in_ECX[0x41] = in_ECX[0x41] | uVar5;
  *(int *)(iVar2 + 0x2504) = in_ECX[0x41];
  *(undefined4 *)(iVar2 + 0x3200) = 0;
  *(undefined4 *)(iVar2 + 0x3250) = 0;
  puVar1 = (uint *)(iVar2 + 0x3204);
  *puVar1 = uVar3 & 0x1f;
  if ((in_ECX[0x41] & uVar5) != 0) {
    *puVar1 = *puVar1 | 0x100;
  }
  *(undefined4 *)(iVar2 + 0x3240) = 0;
  *(undefined4 *)(iVar2 + 0x3244) = 0;
  *(undefined4 *)(iVar2 + 0x322c) = *(undefined4 *)(param_4 + 0xc);
  *(undefined4 *)(iVar2 + 0x3230) = 0;
  *(undefined4 *)(iVar2 + 0x3228) = 0;
  *(undefined4 *)(iVar2 + 0x3280) = 0;
  *(int *)(iVar2 + 0x3224) = iVar4;
  if ((in_ECX[0x41] & uVar5) != 0) {
    *(undefined4 *)(iVar2 + 0x3220) = 1;
  }
  *(undefined4 *)(iVar2 + 0x3250) = 1;
  *(undefined4 *)(iVar2 + 0x3200) = 1;
  *(undefined4 *)(iVar2 + 0x2500) = 1;
  in_ECX[0x42] = in_ECX[0x42] | uVar5;
  return;
}


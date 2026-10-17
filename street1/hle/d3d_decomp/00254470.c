// callers: 0x2550e0

void FUN_00254470(int param_1,uint param_2)

{
  undefined4 *puVar1;
  byte bVar2;
  byte bVar3;
  undefined4 *puVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint *puVar9;
  
  puVar1 = (undefined4 *)(param_1 + 0x1c);
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 0x10) = 0;
  iVar6 = 0x10;
  puVar4 = puVar1;
  do {
    *puVar4 = 2;
    puVar4 = puVar4 + 4;
    iVar6 = iVar6 + -1;
  } while (iVar6 != 0);
  uVar7 = param_2 & 0xe;
  uVar5 = 0;
  if (uVar7 == 4) {
    *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) | 2;
    *puVar1 = 0x42;
    uVar5 = 0x10;
  }
  else if (uVar7 != 0) {
    *puVar1 = 0x32;
    uVar5 = 0xc;
    if (5 < uVar7) {
      iVar6 = (uVar7 - 6 >> 1) + 1;
      *(undefined4 *)(param_1 + 0x28) = 0xc;
      *(uint *)(param_1 + 0x2c) = iVar6 * 0x10 | 2;
      uVar5 = iVar6 * 4 + 0xc;
    }
  }
  if ((param_2 & 0x10) != 0) {
    *(uint *)(param_1 + 0x38) = uVar5;
    *(undefined4 *)(param_1 + 0x3c) = 0x32;
    uVar5 = uVar5 + 0xc;
  }
  if ((param_2 & 0x40) != 0) {
    *(uint *)(param_1 + 0x48) = uVar5;
    *(undefined4 *)(param_1 + 0x4c) = 0x40;
    *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) | 0x400;
    uVar5 = uVar5 + 4;
  }
  if ((char)param_2 < '\0') {
    *(uint *)(param_1 + 0x58) = uVar5;
    *(undefined4 *)(param_1 + 0x5c) = 0x40;
    *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) | 0x800;
    uVar5 = uVar5 + 4;
  }
  uVar8 = param_2 >> 8 & 0xf;
  uVar7 = param_2 >> 0x10;
  if (uVar8 != 0) {
    bVar3 = 0;
    puVar9 = (uint *)(param_1 + 0xac);
    param_2 = uVar8;
    do {
      iVar6 = *(int *)(&DAT_0025f2f8 + (uVar7 & 3) * 4);
      *puVar9 = iVar6 << 4 | 2;
      bVar2 = bVar3 & 0x1f;
      puVar9[-1] = uVar5;
      uVar5 = uVar5 + iVar6 * 4;
      uVar7 = uVar7 >> 2;
      puVar9 = puVar9 + 4;
      bVar3 = bVar3 + 8;
      param_2 = param_2 - 1;
      *(uint *)(param_1 + 0x10) = *(uint *)(param_1 + 0x10) | iVar6 << bVar2;
    } while (param_2 != 0);
  }
  return;
}


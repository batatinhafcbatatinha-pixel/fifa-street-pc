// callers: 0x1934b0 0x194900 0x194e10 0x195c90 0x195db0 0x196480 0x255450 0x25d660 0x25d850 0x25dd90

void FUN_00253800(int param_1,int *param_2)

{
  byte bVar1;
  uint *puVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  uint *puVar6;
  int *piVar7;
  uint uVar8;
  int *piVar9;
  
  puVar6 = DAT_00261238;
  puVar2 = (uint *)DAT_00261238[param_1 + 0x3e2];
  if (puVar2 != (uint *)0x0) {
    uVar3 = *puVar2;
    puVar2[2] = DAT_00261238[0xb];
    *puVar2 = uVar3 - 0x80000;
    if ((uVar3 - 0x80000 & 0x78ffff) == 0) {
      FUN_00257b40(puVar2);
    }
  }
  puVar6[param_1 + 0x3e2] = (uint)param_2;
  if (param_2 == (int *)0x0) {
    piVar7 = (int *)*puVar6;
    if ((int *)puVar6[1] <= piVar7) {
      piVar7 = (int *)FUN_00252040();
    }
    *piVar7 = param_1 * 0x40 + 0x41b0c;
    piVar7[1] = 0;
    *puVar6 = (uint)(piVar7 + 2);
    puVar6[param_1 + 3] = 0x80000000;
    DAT_00260d98 = DAT_00260d98 | 0x4800;
    return;
  }
  *param_2 = *param_2 + 0x80000;
  piVar7 = (int *)*puVar6;
  if ((int *)puVar6[1] <= piVar7) {
    piVar7 = (int *)FUN_00252040();
  }
  iVar4 = param_2[3];
  iVar5 = param_2[1];
  *piVar7 = (param_1 + 0x206c) * 0x40;
  piVar7[1] = iVar5;
  piVar7[2] = iVar4;
  uVar3 = param_2[4];
  piVar9 = piVar7 + 3;
  if (uVar3 != 0) {
    bVar1 = *(byte *)((int)param_2 + 0x13);
    *piVar9 = param_1 * 0x40 + 0x41b10;
    piVar7[4] = (bVar1 + 1) * 0x400000;
    piVar7[5] = param_1 * 0x40 + 0x41b1c;
    piVar7[6] = ((uVar3 & 0xfff) + 1) * 0x10000 | (uVar3 >> 0xc & 0xfff) + 1;
    piVar9 = piVar7 + 7;
  }
  uVar3 = param_2[3];
  uVar8 = uVar3 & 0x20f4;
  if (puVar6[param_1 + 3] != uVar8) {
    if ((char)(uVar8 >> 8) != '\0') {
      uVar8 = uVar3 & 0xf4;
      if ((0x29ff < (uVar3 & 0xff00)) && ((uVar3 & 0xff00) < 0x3101)) {
        uVar8 = uVar8 | 0x40000000;
      }
    }
    puVar6[param_1 + 3] = uVar8;
    DAT_00260d98 = DAT_00260d98 | 0x4000;
    if (puVar2 == (uint *)0x0) {
      uVar3 = puVar6[param_1 + 0x1dd];
      *piVar9 = param_1 * 0x40 + 0x41b0c;
      piVar9[1] = uVar3;
      piVar9 = piVar9 + 2;
      DAT_00260d98 = DAT_00260d98 | 0x800;
    }
  }
  *puVar6 = (uint)piVar9;
  return;
}


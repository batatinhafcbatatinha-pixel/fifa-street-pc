// callers: 0x2578d0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00256310(undefined4 *param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  undefined4 uVar5;
  int iVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  undefined4 *puVar10;
  uint uVar11;
  undefined4 *puVar12;
  
  puVar10 = (undefined4 *)*param_1;
  if ((undefined4 *)param_1[1] <= puVar10) {
    puVar10 = (undefined4 *)FUN_00252040();
  }
  fVar9 = DAT_00261188;
  fVar8 = DAT_00261184;
  fVar7 = DAT_00261180;
  iVar6 = DAT_0026117c;
  fVar4 = DAT_00261174;
  fVar2 = DAT_00261170;
  if (DAT_0026117c == 0) {
    fVar2 = DAT_00261170 * (float)param_1[0x259];
    if (DAT_00261170 * (float)param_1[0x259] < DAT_00261174) {
      fVar2 = DAT_00261174;
    }
    if (DAT_0026118c < fVar2) {
      fVar2 = DAT_0026118c;
    }
    puVar12 = puVar10;
    if (_DAT_0025f4dc < fVar2) {
      fVar2 = _DAT_0025f4dc;
    }
  }
  else {
    fVar1 = DAT_0026118c - DAT_00261174;
    fVar3 = (float)(int)param_1[0x3bb];
    if ((int)param_1[0x3bb] < 0) {
      fVar3 = fVar3 + 4.2949673e+09;
    }
    fVar3 = fVar3 * DAT_00261170;
    *puVar10 = 0x200a30;
    fVar3 = fVar1 / fVar3;
    puVar10[4] = fVar1;
    puVar10[5] = fVar1;
    puVar10[6] = fVar1;
    puVar10[8] = fVar4;
    puVar12 = puVar10 + 9;
    fVar3 = fVar3 * fVar3;
    puVar10[1] = fVar7 * fVar3;
    puVar10[2] = fVar8 * fVar3;
    puVar10[3] = fVar3 * fVar9;
    puVar10[7] = -(fVar4 / fVar1);
  }
  uVar5 = DAT_00261178;
  *puVar12 = &LAB_00080318;
  puVar12[1] = iVar6;
  puVar12[2] = uVar5;
  uVar11 = FUN_002529b0(fVar2 * 8.0 + 0.5);
  if (0x1ff < uVar11) {
    uVar11 = 0x1ff;
  }
  puVar12[3] = 0x4043c;
  puVar12[4] = uVar11;
  *param_1 = puVar12 + 5;
  return;
}


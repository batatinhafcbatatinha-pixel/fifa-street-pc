// callers: 0x2535d0

void FUN_00253d20(uint param_1,int param_2,int *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int *piVar10;
  int *piVar11;
  uint local_c;
  
  puVar4 = DAT_00261238;
  fVar1 = (float)DAT_00261238[599];
  fVar2 = (float)DAT_00261238[600];
  puVar5 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar5) {
    puVar5 = (undefined4 *)FUN_00252040();
  }
  if (param_1 == 0) {
    fVar3 = (float)(int)puVar4[0x3b8];
    if ((int)puVar4[0x3b8] < 0) {
      fVar3 = fVar3 + 4.2949673e+09;
    }
    uVar6 = FUN_002529b0(fVar3 * fVar1 + 0.5);
    fVar3 = (float)(int)puVar4[0x3b9];
    if ((int)puVar4[0x3b9] < 0) {
      fVar3 = fVar3 + 4.2949673e+09;
    }
    local_c = FUN_002529b0(fVar3 * fVar2 + 0.5);
    fVar3 = (float)(int)puVar4[0x3ba];
    if ((int)puVar4[0x3ba] < 0) {
      fVar3 = fVar3 + 4.2949673e+09;
    }
    iVar7 = FUN_002529b0(fVar3 * fVar1 + 0.5);
    fVar1 = (float)(int)puVar4[0x3bb];
    if ((int)puVar4[0x3bb] < 0) {
      fVar1 = fVar1 + 4.2949673e+09;
    }
    iVar8 = FUN_002529b0(fVar1 * fVar2 + 0.5);
  }
  else {
    if ((param_1 != 1) || (param_2 != 0)) {
      iVar7 = puVar4[0x255];
      iVar8 = puVar4[0x256];
      *puVar5 = &LAB_00080200;
      puVar5[1] = iVar7 << 0x10;
      puVar5[2] = iVar8 << 0x10;
      puVar5[3] = 0x402b4;
      puVar5[4] = param_2;
      piVar10 = puVar5 + 5;
      if (param_1 != 0) {
        iVar7 = 0x402e0;
        piVar11 = (int *)((int)piVar10 + (-0x10 - (int)puVar5) + (int)param_3);
        local_c = param_1;
        do {
          iVar8 = piVar11[-1];
          iVar9 = FUN_002529b0((float)piVar11[1] * fVar1 + 0.5);
          uVar6 = FUN_002529b0((float)iVar8 * fVar1 + 0.5);
          *piVar10 = iVar7 + -0x20;
          piVar10[1] = uVar6 | (iVar9 + -1) * 0x10000;
          iVar8 = *piVar11;
          iVar9 = FUN_002529b0((float)piVar11[2] * fVar2 + 0.5);
          uVar6 = FUN_002529b0((float)iVar8 * fVar2 + 0.5);
          piVar10[2] = iVar7;
          piVar10[3] = uVar6 | (iVar9 + -1) * 0x10000;
          piVar10 = piVar10 + 4;
          piVar11 = piVar11 + 4;
          iVar7 = iVar7 + 4;
          local_c = local_c - 1;
        } while (local_c != 0);
      }
      goto LAB_00253fe2;
    }
    uVar6 = FUN_002529b0((float)*param_3 * fVar1 + 0.5);
    local_c = FUN_002529b0((float)param_3[1] * fVar2 + 0.5);
    iVar7 = FUN_002529b0((float)param_3[2] * fVar1 + 0.5);
    iVar7 = iVar7 - uVar6;
    iVar8 = FUN_002529b0((float)param_3[3] * fVar2 + 0.5);
    iVar8 = iVar8 - local_c;
  }
  *puVar5 = &LAB_00080200;
  puVar5[1] = iVar7 << 0x10 | uVar6;
  puVar5[2] = iVar8 << 0x10 | local_c;
  puVar5[3] = 0x402b4;
  puVar5[4] = 0;
  iVar7 = puVar4[0x255];
  puVar5[5] = 0x402c0;
  puVar5[6] = iVar7 << 0x10;
  iVar7 = puVar4[0x256];
  puVar5[7] = 0x402e0;
  puVar5[8] = iVar7 << 0x10;
  piVar10 = puVar5 + 9;
LAB_00253fe2:
  *puVar4 = piVar10;
  piVar10 = puVar4 + 0x6e0;
  for (iVar7 = (param_1 & 0xfffffff) << 2; iVar7 != 0; iVar7 = iVar7 + -1) {
    *piVar10 = *param_3;
    param_3 = param_3 + 1;
    piVar10 = piVar10 + 1;
  }
  for (iVar7 = 0; iVar7 != 0; iVar7 = iVar7 + -1) {
    *(char *)piVar10 = (char)*param_3;
    param_3 = (int *)((int)param_3 + 1);
    piVar10 = (int *)((int)piVar10 + 1);
  }
  puVar4[0x700] = param_1;
  puVar4[0x701] = param_2;
  return;
}


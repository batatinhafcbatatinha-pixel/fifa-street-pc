// callers: 0x1a06a0

void FUN_0025b070(int *param_1,int param_2)

{
  uint *puVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  uint *puVar8;
  uint uVar9;
  uint uVar10;
  uint local_2c;
  uint local_28;
  uint *local_20;
  uint local_14 [3];
  uint local_8;
  uint local_4;
  
  piVar2 = DAT_00261238;
  local_2c = 0;
  FUN_00257950(0);
  if (*param_1 < 0) {
    uVar10 = param_1[3] - 4U >> 2;
    iVar3 = FUN_00252050(uVar10);
    FUN_0025c9e0(iVar3,param_1[1],uVar10);
    *piVar2 = iVar3 + uVar10 * 4;
    return;
  }
  uVar10 = param_1[1];
  iVar3 = param_1[3] + -4;
  local_28 = 0;
  if (param_2 != 0) {
    local_2c = 0x20000000;
    local_28 = *(int *)(param_2 + 0xc) + *(int *)(param_2 + 4);
  }
  piVar4 = (int *)*piVar2;
  if ((*(byte *)(piVar2 + 2) & 4) != 0) {
    if ((int *)piVar2[1] <= piVar4) {
      piVar4 = (int *)FUN_00252040();
    }
    *piVar4 = 0x40100;
    piVar4[1] = (int)(piVar4 + 3) * 0x20 | 0xe;
    piVar4[2] = (uVar10 & 0xfffffff) + 1;
    piVar4[3] = (uint)(piVar4 + 8) & 0xfffffff | local_2c;
    piVar4[4] = 0;
    piVar4[5] = iVar3 + uVar10;
    piVar4[6] = local_28;
    piVar4[7] = uVar10;
    *piVar2 = (int)(piVar4 + 8);
    goto LAB_0025b2c2;
  }
  if ((int *)piVar2[1] <= piVar4) {
    piVar4 = (int *)FUN_00252040();
  }
  if ((uint)(piVar2[0xb] - param_1[2]) < (uint)(piVar2[0xb] - *(int *)piVar2[0xc])) {
    *piVar4 = 0x40100;
    piVar4[1] = 0;
    piVar4[2] = (uVar10 & 0xfffffff) + 1;
    puVar1 = (uint *)(piVar4 + 3);
    uVar5 = piVar2[0x13] & 0x3f;
    if ((piVar2[uVar5 * 5 + 0x99] & 0x40000000U) == 0) {
      uVar7 = (uint)puVar1 & 0xfffffff | 0xc0000000;
      uVar9 = uVar5 << 5 | 0xd;
      piVar2[0x13] = uVar5 + 1;
      puVar8 = (uint *)(piVar2 + uVar5 * 5 + 0x99);
      local_20 = puVar1;
    }
    else {
      local_20 = (uint *)(piVar4 + 8);
      uVar7 = (uint)local_20 & 0xfffffff;
      uVar9 = (int)puVar1 * 0x20 | 0xc;
      puVar8 = puVar1;
    }
    local_2c = local_2c | uVar7;
    *puVar8 = local_2c;
    puVar8[1] = 0;
    puVar8[2] = uVar10 + iVar3;
    puVar8[3] = local_28;
    puVar8[4] = uVar10;
    if ((piVar2[0x14] == 0) || (param_1[5] == piVar2[0x15])) {
LAB_0025b21f:
      piVar2[0x14] = piVar2[0x14] + 1;
      piVar2[0x15] = piVar2[0x15] + 1;
      piVar4[1] = uVar9;
    }
    else {
      iVar6 = *(int *)(piVar2[0x16] + 4);
      if (iVar6 == 0) {
        *(int *)(piVar2[0x16] + 4) = (int)puVar8;
        iVar6 = 0;
      }
      if (iVar6 != 0) goto LAB_0025b21f;
    }
    param_1[5] = piVar2[0x15];
    piVar2[0x16] = (int)puVar8;
  }
  else {
    *piVar4 = (uVar10 & 0xfffffff) + 1;
    local_20 = (uint *)(piVar4 + 1);
    local_14[0] = (uint)local_20 & 0xfffffff | local_2c;
    local_14[1] = 0;
    local_8 = local_28;
    local_14[2] = iVar3 + uVar10;
    local_4 = uVar10;
    FUN_0025afd0(local_14);
    param_1[5] = piVar2[0x15];
  }
  iVar6 = piVar2[0xf];
  *piVar2 = (int)local_20;
  piVar2[0xf] = iVar6 + iVar3;
  if (DAT_002639e8 < (uint)(iVar6 + iVar3)) {
    FUN_00251cd0(0);
  }
LAB_0025b2c2:
  param_1[2] = piVar2[0xb];
  if (param_2 == 0) {
    return;
  }
  *(int *)(param_2 + 8) = piVar2[0xb];
  return;
}


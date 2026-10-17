
void FUN_001954e0(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  int in_ECX;
  int *piVar4;
  int iVar5;
  
  iVar1 = FUN_001952b0(param_1);
  iVar5 = 0;
  *(int *)(in_ECX + 0x40) = iVar1;
  if (iVar1 == 0) {
    param_1 = (int *)(DAT_002d51f4 + 0x2d51e0);
    uVar2 = FUN_001952b0(param_1);
    *(undefined4 *)(in_ECX + 0x40) = uVar2;
  }
  piVar4 = (int *)(*(int *)(in_ECX + 0x40) + 4);
  *piVar4 = *piVar4 + 1;
  if (*(int *)(*(int *)(in_ECX + 0x40) + 4) != 1) {
    *(undefined4 *)(in_ECX + 0x3c) = 0;
    for (; param_1 != (int *)0x0; param_1 = (int *)((int)param_1 + iVar1)) {
      if ((char)*param_1 == '*') goto LAB_00195693;
      iVar1 = *param_1 >> 8;
      if (iVar1 == 0) break;
    }
    param_1 = (int *)0x0;
LAB_00195693:
    *(int **)(in_ECX + 0x3c) = param_1;
    if (param_1 != (int *)0x0) {
      if ((param_1[3] & 0x1000U) == 0) {
        param_1 = param_1 + 4;
      }
      else {
        param_1 = (int *)(param_1[4] + (int)param_1);
      }
      *(int **)(in_ECX + 0x3c) = param_1;
      if (*(int *)(in_ECX + 0x38) == 0) {
        uVar2 = (*(code *)PTR_FUN_002d4d38)(0xc,"D3DPalette");
        *(undefined4 *)(in_ECX + 0x38) = uVar2;
      }
      else {
        iVar1 = 0;
        do {
          if ((&DAT_002f3470)[iVar1] == *(int *)(in_ECX + 0x38)) {
            FUN_002539b0(iVar1,0);
            (&DAT_002f3470)[iVar1] = 0;
          }
          iVar1 = iVar1 + 1;
        } while (iVar1 < 4);
        thunk_FUN_002520c0(*(undefined4 *)(in_ECX + 0x38));
      }
      FUN_00265aa8(0,*(undefined4 *)(in_ECX + 0x38),0);
      FUN_00257e10(*(undefined4 *)(in_ECX + 0x38),*(undefined4 *)(in_ECX + 0x3c));
    }
    uVar3 = FUN_00239875(*(undefined4 *)(*(int *)(in_ECX + 0x40) + 0x18));
    if ((uVar3 & 2) != 0) {
      *(undefined4 *)(*(int *)(in_ECX + 0x40) + 0x30) = 0xe0000000;
    }
    if (((((*(byte *)(*(int *)(in_ECX + 0x40) + 0x13) & 1) == 0) &&
         (iVar1 = *(int *)(*(int *)(in_ECX + 0x40) + 0x14), iVar1 != 0xc)) && (iVar1 != 0xe)) &&
       (iVar1 != 0xf)) {
      *(undefined4 *)(in_ECX + 4) = 3;
      *(undefined4 *)(in_ECX + 8) = 3;
      *(undefined4 *)(in_ECX + 0xc) = 3;
      *(undefined4 *)(in_ECX + 0x10) = 3;
    }
    return;
  }
  *(int **)(*(int *)(in_ECX + 0x40) + 0x18) = param_1;
  *(int *)(*(int *)(in_ECX + 0x40) + 8) = (int)(short)param_1[1];
  *(int *)(*(int *)(in_ECX + 0x40) + 0xc) = (int)*(short *)((int)param_1 + 6);
  uVar3 = *(uint *)(*(int *)(in_ECX + 0x40) + 0x10);
  *(uint *)(*(int *)(in_ECX + 0x40) + 0x10) =
       uVar3 ^ (((uint)param_1[3] >> 0x1c) + 1 ^ uVar3) & 0x3f;
  uVar3 = *(uint *)(*(int *)(in_ECX + 0x40) + 0x10);
  *(uint *)(*(int *)(in_ECX + 0x40) + 0x10) = uVar3 ^ (param_1[3] << 0xb ^ uVar3) & 0x1000000;
  iVar1 = FUN_0016a8fb(param_1);
  uVar3 = *(uint *)(*(int *)(in_ECX + 0x40) + 0x10);
  *(uint *)(*(int *)(in_ECX + 0x40) + 0x10) = uVar3 ^ (iVar1 << 6 ^ uVar3) & 0x3fc0;
  uVar2 = FUN_001946c0(param_1);
  *(undefined4 *)(*(int *)(in_ECX + 0x40) + 0x14) = uVar2;
  *(undefined4 *)(in_ECX + 0x3c) = 0;
  piVar4 = param_1;
  do {
    if ((char)*piVar4 == '*') goto LAB_001955b3;
    iVar1 = *piVar4 >> 8;
  } while ((iVar1 != 0) && (piVar4 = (int *)((int)piVar4 + iVar1), piVar4 != (int *)0x0));
  piVar4 = (int *)0x0;
LAB_001955b3:
  *(int **)(in_ECX + 0x3c) = piVar4;
  if (piVar4 != (int *)0x0) {
    if ((piVar4[3] & 0x1000U) == 0) {
      piVar4 = piVar4 + 4;
    }
    else {
      piVar4 = (int *)(piVar4[4] + (int)piVar4);
    }
    *(int **)(in_ECX + 0x3c) = piVar4;
    if (*(int *)(in_ECX + 0x38) == 0) {
      uVar2 = (*(code *)PTR_FUN_002d4d38)(0xc,"D3DPalette");
      *(undefined4 *)(in_ECX + 0x38) = uVar2;
    }
    else {
      do {
        if ((&DAT_002f3470)[iVar5] == *(int *)(in_ECX + 0x38)) {
          FUN_002539b0(iVar5,0);
          (&DAT_002f3470)[iVar5] = 0;
        }
        iVar5 = iVar5 + 1;
      } while (iVar5 < 4);
      thunk_FUN_002520c0(*(undefined4 *)(in_ECX + 0x38));
    }
    FUN_00265aa8(0,*(undefined4 *)(in_ECX + 0x38),0);
    FUN_00257e10(*(undefined4 *)(in_ECX + 0x38),*(undefined4 *)(in_ECX + 0x3c));
  }
  if ((param_1[3] & 0x1000U) != 0) {
    *(int *)(*(int *)(in_ECX + 0x40) + 0x1c) = param_1[4] + (int)param_1;
    FUN_00194e10();
    return;
  }
  *(int **)(*(int *)(in_ECX + 0x40) + 0x1c) = param_1 + 4;
  FUN_00194e10();
  return;
}


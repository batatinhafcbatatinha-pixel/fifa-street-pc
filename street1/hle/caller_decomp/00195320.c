
undefined4 FUN_00195320(int *param_1)

{
  int iVar1;
  int in_ECX;
  
  if (param_1 == (int *)0x0) {
    return 0;
  }
  while ((char)*param_1 != '*') {
    iVar1 = *param_1 >> 8;
    if ((iVar1 == 0) || (param_1 = (int *)((int)param_1 + iVar1), param_1 == (int *)0x0)) {
      return 0;
    }
  }
  if ((param_1[3] & 0x1000U) == 0) {
    param_1 = param_1 + 4;
  }
  else {
    param_1 = (int *)(param_1[4] + (int)param_1);
  }
  *(int **)(in_ECX + 0x3c) = param_1;
  if ((param_1 != (int *)0x0) && (*(int *)(in_ECX + 0x38) != 0)) {
    iVar1 = 0;
    do {
      if ((&DAT_002f3470)[iVar1] == *(int *)(in_ECX + 0x38)) {
        FUN_002539b0(iVar1,0);
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 < 4);
    thunk_FUN_002520c0(*(undefined4 *)(in_ECX + 0x38));
    FUN_00265aa8(0,*(undefined4 *)(in_ECX + 0x38),0);
    FUN_00257e10(*(undefined4 *)(in_ECX + 0x38),param_1);
    iVar1 = 0;
    do {
      if ((&DAT_002f3470)[iVar1] == *(int *)(in_ECX + 0x38)) {
        FUN_002539b0(iVar1,*(int *)(in_ECX + 0x38));
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 < 4);
    return 1;
  }
  return 0;
}



void FUN_00195db0(int param_1)

{
  int *piVar1;
  int iVar2;
  int *in_ECX;
  int iVar3;
  
  piVar1 = (int *)(*(int *)(*in_ECX + 0x40) + 4);
  *piVar1 = *piVar1 + -1;
  if (*(int *)(*(int *)(*in_ECX + 0x40) + 4) == 0) {
    iVar3 = 0;
    do {
      if ((&DAT_002f3450)[iVar3] == *(int *)(*(int *)(*in_ECX + 0x40) + 0x24)) {
        FUN_00253800(iVar3,0);
        (&DAT_002f3450)[iVar3] = 0;
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < 4);
    iVar3 = *(int *)(*(int *)(*in_ECX + 0x40) + 0x24);
    if (iVar3 != 0) {
      thunk_FUN_002520c0(iVar3);
    }
    iVar3 = ((int *)*in_ECX)[0xe];
    if (iVar3 != 0) {
      iVar2 = *(int *)*in_ECX;
      if ((&DAT_002f3470)[iVar2] == iVar3) {
        FUN_002539b0(iVar2,0);
        (&DAT_002f3470)[*(int *)*in_ECX] = 0;
      }
      thunk_FUN_002520c0(*(undefined4 *)(*in_ECX + 0x38));
    }
    iVar3 = *(int *)(*in_ECX + 0x40);
    if ((*(uint *)(iVar3 + 0x10) & 0x800000) == 0) {
      FUN_001958d0();
    }
    else if (iVar3 != 0) {
      FUN_001958d0();
      (*(code *)PTR_FUN_002d4d3c)(iVar3,0x34);
    }
  }
  piVar1 = (int *)(*(int *)(param_1 + 0x40) + 4);
  *piVar1 = *piVar1 + 1;
  *(undefined4 *)(*in_ECX + 0x40) = *(undefined4 *)(param_1 + 0x40);
  *(undefined4 *)(*in_ECX + 0x38) = *(undefined4 *)(param_1 + 0x38);
  *(undefined4 *)(*in_ECX + 0x3c) = *(undefined4 *)(param_1 + 0x3c);
  return;
}


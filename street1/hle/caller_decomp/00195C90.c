
void FUN_00195c90(int param_1)

{
  int *piVar1;
  uint *puVar2;
  undefined4 uVar3;
  int *in_ECX;
  int iVar4;
  
  if (param_1 == 0) {
    param_1 = DAT_002d51f4 + 0x2d51e0;
  }
  piVar1 = (int *)(*(int *)(*in_ECX + 0x40) + 4);
  *piVar1 = *piVar1 + -1;
  if (*(int *)(*(int *)(*in_ECX + 0x40) + 4) == 0) {
    iVar4 = 0;
    do {
      if ((&DAT_002f3450)[iVar4] == *(int *)(*(int *)(*in_ECX + 0x40) + 0x24)) {
        FUN_00253800(iVar4,0);
        (&DAT_002f3450)[iVar4] = 0;
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < 4);
    iVar4 = *(int *)(*(int *)(*in_ECX + 0x40) + 0x24);
    if (iVar4 != 0) {
      thunk_FUN_002520c0(iVar4);
    }
    iVar4 = *(int *)(*in_ECX + 0x40);
    if ((*(uint *)(iVar4 + 0x10) & 0x800000) == 0) {
      FUN_001958d0();
    }
    else if (iVar4 != 0) {
      FUN_001958d0();
      (*(code *)PTR_FUN_002d4d3c)(iVar4,0x34);
    }
  }
  uVar3 = FUN_001952b0(param_1);
  *(undefined4 *)(*in_ECX + 0x40) = uVar3;
  if (*(int *)(*in_ECX + 0x40) == 0) {
    iVar4 = (*(code *)PTR_FUN_002d4d38)(0x34,0);
    if (iVar4 == 0) {
      iVar4 = 0;
    }
    else {
      *(uint *)(iVar4 + 0x10) = *(uint *)(iVar4 + 0x10) & 0xff7fffff | 0x400000;
      *(undefined4 *)(iVar4 + 4) = 0;
      *(undefined4 *)(iVar4 + 8) = 0;
      *(undefined4 *)(iVar4 + 0xc) = 0;
      *(undefined4 *)(iVar4 + 0x18) = 0;
      *(undefined4 *)(iVar4 + 0x20) = 0;
      *(undefined4 *)(iVar4 + 0x24) = 0;
      *(uint *)(iVar4 + 0x28) = *(uint *)(iVar4 + 0x28) & 0xfffffffe;
      *(undefined1 *)(iVar4 + 0x2c) = 0;
      *(undefined4 *)(iVar4 + 0x30) = 0;
    }
    *(int *)(*in_ECX + 0x40) = iVar4;
    puVar2 = (uint *)(*(int *)(*in_ECX + 0x40) + 0x10);
    *puVar2 = *puVar2 | 0x800000;
  }
  FUN_001954e0(param_1);
  return;
}


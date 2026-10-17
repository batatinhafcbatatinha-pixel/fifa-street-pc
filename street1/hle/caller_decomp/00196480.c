
void FUN_00196480(void)

{
  int *in_ECX;
  int iVar1;
  
  *(int *)(in_ECX[0x10] + 4) = *(int *)(in_ECX[0x10] + 4) + -1;
  if (*(int *)(in_ECX[0x10] + 4) == 0) {
    iVar1 = 0;
    do {
      if ((&DAT_002f3450)[iVar1] == *(int *)(in_ECX[0x10] + 0x24)) {
        FUN_00253800(iVar1,0);
        (&DAT_002f3450)[iVar1] = 0;
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 < 4);
    if (*(int *)(in_ECX[0x10] + 0x24) != 0) {
      thunk_FUN_002520c0(*(int *)(in_ECX[0x10] + 0x24));
    }
    if ((*(byte *)(in_ECX[0x10] + 0x28) & 1) != 0) {
      (*(code *)PTR_FUN_002d4d3c)(*(undefined4 *)(in_ECX[0x10] + 0x18),0x14);
    }
    iVar1 = in_ECX[0x10];
    if ((*(uint *)(iVar1 + 0x10) & 0x800000) == 0) {
      FUN_001958d0();
    }
    else if (iVar1 != 0) {
      FUN_001958d0();
      (*(code *)PTR_FUN_002d4d3c)(iVar1,0x34);
    }
  }
  if (in_ECX[0xe] != 0) {
    if ((&DAT_002f3470)[*in_ECX] == in_ECX[0xe]) {
      FUN_002539b0(*in_ECX,0);
      (&DAT_002f3470)[*in_ECX] = 0;
    }
    thunk_FUN_002520c0(in_ECX[0xe]);
    (*(code *)PTR_FUN_002d4d3c)(in_ECX[0xe],0xc);
    in_ECX[0xe] = 0;
  }
  return;
}


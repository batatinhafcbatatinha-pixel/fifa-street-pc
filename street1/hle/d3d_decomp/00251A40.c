// callers: 0x251d90

uint FUN_00251a40(uint *param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  if (param_2 != 0) {
    do {
    } while (((*(int *)param_1[0xc] << 2 ^ *(uint *)(param_1[0x24d] + 0x400b10)) & 0x7c) != 0);
  }
  uVar1 = *(uint *)(param_1[0x708] + 0x44) | 0x80000000;
  if ((uVar1 < param_1[9]) || (param_1[10] <= uVar1)) {
    if (*(int *)param_1[0xc] + 2U == param_1[0xb]) {
      return *param_1;
    }
    iVar2 = FUN_002518c0();
    if (iVar2 != 0) {
      return *(uint *)(iVar2 + 4);
    }
    uVar1 = *param_1 + 4;
  }
  return uVar1;
}


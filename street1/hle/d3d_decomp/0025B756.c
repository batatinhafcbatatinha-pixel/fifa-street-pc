// callers: 0x25c5b8

void FUN_0025b756(int param_1)

{
  int iVar1;
  int *in_ECX;
  
  iVar1 = *in_ECX;
  if (param_1 != 2) {
    *(uint *)(iVar1 + 0x400784) = in_ECX[param_1 + 0x4e] & 0xffff;
    *(undefined4 *)(iVar1 + 0x400788) = 2;
    FUN_0025b717();
    *(undefined4 *)(iVar1 + 0x400144) = 0x10000000;
  }
  return;
}


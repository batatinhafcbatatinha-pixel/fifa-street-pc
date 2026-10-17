// callers: 0x25a1f4

undefined4 FUN_00259d2c(void)

{
  int iVar1;
  int *in_ECX;
  
  iVar1 = *in_ECX;
  in_ECX[0x2a] = *(uint *)(iVar1 + 0x1800) >> 0x10 & 0xfffc;
  in_ECX[0x30] = *(uint *)(iVar1 + 0x1808) & 0xff;
  in_ECX[0x2b] = *(int *)(iVar1 + 0x10020c);
  in_ECX[0x2f] = 0xfe502a;
  return 1;
}


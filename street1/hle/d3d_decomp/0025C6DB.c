// callers: 0x259df0

void FUN_0025c6db(void)

{
  int iVar1;
  int *in_ECX;
  
  iVar1 = *in_ECX;
  *(undefined **)(iVar1 + 0x3224) = &DAT_000f0078;
  *(undefined4 *)(iVar1 + 0x2044) = 0x101ffff;
  *(uint *)(iVar1 + 0x2040) = in_ECX[0x48] & 0x3ff;
  *(undefined4 *)(iVar1 + 0x2500) = 0;
  *(undefined4 *)(iVar1 + 0x3000) = 0;
  *(undefined4 *)(iVar1 + 0x3050) = 0;
  *(undefined4 *)(iVar1 + 0x3200) = 0;
  *(undefined4 *)(iVar1 + 0x3250) = 0;
  *(undefined4 *)(iVar1 + 0x3220) = 0;
  FUN_0025b78f(1);
  *(undefined4 *)(iVar1 + 0x3210) = 0;
  *(undefined4 *)(iVar1 + 0x3270) = 0;
  *(undefined4 *)(iVar1 + 0x3250) = 1;
  *(undefined4 *)(iVar1 + 0x3200) = 1;
  *(undefined4 *)(iVar1 + 0x2500) = 1;
  *(undefined4 *)(iVar1 + 0x2500) = 0;
  return;
}


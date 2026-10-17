// callers: 0x255bf0

uint FUN_002517f0(void)

{
  int iVar1;
  uint uVar2;
  byte bVar3;
  int *in_ECX;
  
  iVar1 = FUN_001609a7(DAT_002639ec,0xbc800000);
  in_ECX[9] = iVar1;
  if (iVar1 == 0) {
    return 0x8007000e;
  }
  in_ECX[10] = iVar1 + (DAT_002639ec & 0xfffffffc);
  *in_ECX = iVar1;
  in_ECX[1] = iVar1 + -0x204 + (DAT_002639e8 & 0xfffffffc);
  in_ECX[0xb] = 5;
  *(undefined4 *)in_ECX[0xc] = 3;
  uVar2 = (DAT_002639ec - 1) / DAT_002639e8;
  bVar3 = 0;
  do {
    uVar2 = uVar2 >> 1;
    bVar3 = bVar3 + 1;
  } while (uVar2 != 0);
  iVar1 = 1 << (bVar3 & 0x1f);
  in_ECX[0xe] = iVar1 + -1;
  iVar1 = FUN_001609a7(iVar1 * 8,0x64800000);
  in_ECX[0x12] = iVar1;
  return (iVar1 != 0) - 1 & 0x8007000e;
}


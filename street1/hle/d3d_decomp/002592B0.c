// callers: 0x259390 0x259690

int FUN_002592b0(void)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int *in_ECX;
  int *piVar4;
  int iVar5;
  
  iVar2 = *in_ECX;
  uVar3 = in_ECX[0x6f] & 1;
  iVar1 = uVar3 * 3 + 0x5d;
  piVar4 = in_ECX + iVar1;
  iVar5 = 0;
  if (in_ECX[iVar1] == 0) {
    return 0;
  }
  do {
    if (in_ECX[0x70] != in_ECX[uVar3 * 3 + 0x5e]) {
      return iVar5;
    }
    *piVar4 = 0;
    DAT_002639f4 = in_ECX[uVar3 * 3 + 0x5f];
    FUN_00257eb2(DAT_002639f4);
    uVar3 = in_ECX[0x6f] & 1;
    if (in_ECX[uVar3 + 0x1f7] == 1) {
      FUN_00257f34(in_ECX + uVar3 * 0xc0 + 0x77);
      in_ECX[uVar3 + 0x1f7] = 0;
    }
    *(uint *)(iVar2 + 0x40071c) = *(uint *)(iVar2 + 0x40071c) | 2;
    uVar3 = in_ECX[0x6f] + 1U & 1;
    in_ECX[0x6f] = in_ECX[0x6f] + 1U;
    piVar4 = in_ECX + uVar3 * 3 + 0x5d;
    iVar5 = iVar5 + 1;
  } while (*piVar4 != 0);
  return iVar5;
}


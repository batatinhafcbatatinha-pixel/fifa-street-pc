// callers: 

void FUN_00251c60(void)

{
  int iVar1;
  uint *in_ECX;
  uint uVar2;
  
  if ((in_ECX[2] & 4) == 0) {
    uVar2 = *in_ECX;
  }
  else {
    uVar2 = in_ECX[0x1dc];
  }
  if ((in_ECX[2] & 0x2000) == 0) {
    iVar1 = *(int *)(DAT_00261238 + 0x1c28);
    *(uint *)(iVar1 + 0x100410) = *(uint *)(iVar1 + 0x100410) | 0x10000;
    do {
    } while ((*(uint *)(iVar1 + 0x100410) & 0x10000) != 0);
    *(uint *)(in_ECX[0x708] + 0x40) = uVar2 & 0xfffffff;
    do {
      iVar1 = FUN_00253ad0();
    } while (iVar1 != 0);
  }
  return;
}


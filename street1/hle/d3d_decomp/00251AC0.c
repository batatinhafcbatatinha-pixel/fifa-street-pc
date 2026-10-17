// callers: 0x251d90 0x259690

void FUN_00251ac0(void)

{
  int iVar1;
  
  iVar1 = *(int *)(DAT_00261238 + 0x1c28);
  *(uint *)(iVar1 + 0x100410) = *(uint *)(iVar1 + 0x100410) | 0x10000;
  do {
  } while ((*(uint *)(iVar1 + 0x100410) & 0x10000) != 0);
  return;
}


// callers: 0x251bb0 0x251c60 0x255bf0

bool FUN_00253ad0(void)

{
  int iVar1;
  
  iVar1 = *(int *)(DAT_00261238 + 0x934);
  if (*(int *)(iVar1 + 0x3240) != *(int *)(iVar1 + 0x3244)) {
    return true;
  }
  return *(int *)(iVar1 + 0x400700) != 0;
}


// callers: 0x2560c0

void FUN_00251890(void)

{
  int in_ECX;
  
  if (*(int *)(in_ECX + 0x24) != 0) {
    FUN_00160a47(*(int *)(in_ECX + 0x24),0xa1800000);
  }
  if (*(int *)(in_ECX + 0x48) != 0) {
    FUN_00160a47(*(int *)(in_ECX + 0x48),0x24800000);
  }
  return;
}


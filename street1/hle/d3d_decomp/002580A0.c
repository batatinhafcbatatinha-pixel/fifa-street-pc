// callers: 0x258870

int FUN_002580a0(void)

{
  int iVar1;
  uint in_ECX;
  
  iVar1 = 0;
  if (in_ECX != 0) {
    for (; (in_ECX >> iVar1 & 1) == 0; iVar1 = iVar1 + 1) {
    }
  }
  return iVar1;
}


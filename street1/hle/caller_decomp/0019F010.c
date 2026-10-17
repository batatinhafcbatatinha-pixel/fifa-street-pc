
void FUN_0019f010(void)

{
  int *in_ECX;
  int iVar1;
  
  iVar1 = 0;
  do {
    if ((&DAT_002f49a0)[iVar1] == *in_ECX) {
      (&DAT_002f49a0)[iVar1] = 0;
      FUN_00254ce0(iVar1,0,0);
    }
    iVar1 = iVar1 + 1;
  } while (iVar1 < 0x10);
  thunk_FUN_002520c0(*in_ECX);
  if ((char)in_ECX[3] != '\0') {
    (*(code *)PTR_FUN_002d4d3c)(*in_ECX,0xc);
    *in_ECX = 0;
    return;
  }
  FUN_00257ca0(*in_ECX);
  *in_ECX = 0;
  return;
}


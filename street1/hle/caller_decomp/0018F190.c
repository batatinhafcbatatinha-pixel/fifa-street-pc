
void FUN_0018f190(void)

{
  int in_ECX;
  int iVar1;
  
  if (*(char *)(in_ECX + 0x5c) != '\0') {
    if (*(int *)(in_ECX + 0xa8) == 0x60) {
      **(undefined4 **)(in_ECX + 0xa0) = 0x417fc;
      *(undefined4 *)(*(int *)(in_ECX + 0xa0) + 4) = 0;
      **(int **)(in_ECX + 0xa4) = *(int *)(in_ECX + 0xa8) * 0x180000 + 0x40001818;
    }
    else {
      **(undefined4 **)(in_ECX + 0xa0) = 0x417fc;
      *(undefined4 *)(*(int *)(in_ECX + 0xa0) + 4) = 0;
    }
    iVar1 = *(int *)(in_ECX + 0xa0) + 8;
    *(int *)(in_ECX + 0xa0) = iVar1;
    FUN_00251ba0(iVar1);
    *(undefined4 *)(in_ECX + 0xa8) = 0;
  }
  return;
}


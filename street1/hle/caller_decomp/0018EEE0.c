
void FUN_0018eee0(void)

{
  int in_ECX;
  int iVar1;
  
  if (*(char *)(in_ECX + 0x58) != '\0') {
    if (*(int *)(in_ECX + 0xa4) == 0x60) {
      **(undefined4 **)(in_ECX + 0x9c) = 0x417fc;
      *(undefined4 *)(*(int *)(in_ECX + 0x9c) + 4) = 0;
      **(int **)(in_ECX + 0xa0) = *(int *)(in_ECX + 0xa4) * 0x140000 + 0x40001818;
    }
    else {
      **(undefined4 **)(in_ECX + 0x9c) = 0x417fc;
      *(undefined4 *)(*(int *)(in_ECX + 0x9c) + 4) = 0;
    }
    iVar1 = *(int *)(in_ECX + 0x9c) + 8;
    *(int *)(in_ECX + 0x9c) = iVar1;
    FUN_00251ba0(iVar1);
    *(undefined4 *)(in_ECX + 0xa4) = 0;
  }
  return;
}


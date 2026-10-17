// callers: 0x19f310

void FUN_00254fd0(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + -1) + -1;
  *(int *)(param_1 + -1) = iVar1;
  if (iVar1 == 0) {
    FUN_00160a47((int *)(param_1 + -1),0x24800000);
  }
  return;
}


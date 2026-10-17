// callers: 0x1a0700

void FUN_00254ca0(int param_1)

{
  int in_ECX;
  int iVar1;
  undefined4 *in_EDX;
  undefined4 *puVar2;
  
  if (((byte)DAT_00261248 & 0x10) == 0) {
    puVar2 = (undefined4 *)(&DAT_00260180 + in_ECX * 0x10);
    for (iVar1 = param_1; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = *in_EDX;
      in_EDX = in_EDX + 1;
      puVar2 = puVar2 + 1;
    }
  }
  FUN_00254bc0(param_1);
  return;
}


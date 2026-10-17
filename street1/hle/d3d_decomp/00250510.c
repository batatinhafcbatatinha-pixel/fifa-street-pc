// callers: 0x25dec0

void FUN_00250510(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  
  puVar1 = DAT_00261238;
  if (DAT_00261238[0x1e1] == 0) {
    puVar2 = (undefined4 *)*DAT_00261238;
    if ((undefined4 *)DAT_00261238[1] <= puVar2) {
      puVar2 = (undefined4 *)FUN_00252040();
    }
    *puVar2 = 0x400a60;
    puVar2 = puVar2 + 1;
    iVar3 = 0x10;
    do {
      *puVar2 = param_1;
      puVar2 = puVar2 + 1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    *puVar1 = puVar2;
    DAT_002611f0 = param_1;
    return;
  }
  DAT_002611f0 = param_1;
  return;
}


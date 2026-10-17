// callers: 0x1934b0

void FUN_00250740(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  uVar3 = DAT_002611d0;
  if (DAT_002611d4 == 0) {
    uVar3 = param_1;
  }
  *puVar2 = &DAT_0008038c;
  puVar2[1] = param_1;
  puVar2[2] = uVar3;
  *puVar1 = puVar2 + 3;
  DAT_002611cc = param_1;
  return;
}


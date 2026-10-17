// callers: 0x25a356 0x25a544

void FUN_0025a041(undefined4 *param_1)

{
  byte *pbVar1;
  int iVar2;
  int *in_ECX;
  
  iVar2 = *in_ECX;
  while ((((*(byte *)(iVar2 + 0x3214) & 0x10) == 0 || ((*(byte *)(iVar2 + 0x2400) & 0x10) == 0)) ||
         ((*(byte *)(iVar2 + 0x3220) & 0x10) != 0))) {
    FUN_002599b0();
    if (*(int *)(iVar2 + 0x400100) != 0) {
      FUN_00259900();
    }
    if ((*(uint *)(iVar2 + 0x100) & 0x1000000) != 0) {
      FUN_00259390();
    }
  }
  pbVar1 = (byte *)(iVar2 + 0x3220);
  *param_1 = *(undefined4 *)pbVar1;
  pbVar1[0] = 0;
  pbVar1[1] = 0;
  pbVar1[2] = 0;
  pbVar1[3] = 0;
  do {
  } while ((*pbVar1 & 0x10) != 0);
  return;
}


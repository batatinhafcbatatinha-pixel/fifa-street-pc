// callers: 0x1934b0 0x198750

void FUN_00250420(int param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  bool bVar3;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x40308;
  if (param_1 == 0) {
    puVar2[1] = 0;
    *puVar1 = puVar2 + 2;
    DAT_002611ec = param_1;
    return;
  }
  puVar2[1] = 1;
  bVar3 = param_1 != DAT_002611e8;
  puVar2[2] = 0x4039c;
  puVar2[3] = bVar3 + 0x404;
  *puVar1 = puVar2 + 4;
  DAT_002611ec = param_1;
  return;
}


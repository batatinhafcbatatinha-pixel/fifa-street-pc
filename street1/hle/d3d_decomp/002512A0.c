// callers: 0x198750

void FUN_002512a0(int param_1)

{
  uint *puVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined4 uVar4;
  bool bVar5;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  if ((param_1 == 0) || (puVar1[0x682] == 0)) {
    uVar4 = 0;
  }
  else {
    uVar4 = 1;
  }
  *puVar2 = 0x4030c;
  puVar2[1] = uVar4;
  uVar4 = DAT_002610e8;
  puVar2[2] = 0x41d78;
  puVar2[3] = uVar4;
  *puVar1 = (uint)(puVar2 + 4);
  bVar5 = DAT_002611dc == 2;
  DAT_002611dc = param_1;
  if ((bVar5) || (param_1 == 2)) {
    FUN_00250210();
    FUN_00254ff0();
    if (puVar1[1] <= *puVar1) {
      FUN_00252040();
    }
    FUN_00250e10();
    uVar3 = FUN_00250c20();
    *puVar1 = uVar3;
  }
  return;
}


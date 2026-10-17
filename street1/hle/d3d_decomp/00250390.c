// callers: 0x198750

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00250390(int param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x41e6c;
  puVar2[1] = param_1 + -0x200;
  *puVar1 = puVar2 + 2;
  _DAT_00261210 = param_1;
  return;
}


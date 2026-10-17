// callers: 0x199cf0

void FUN_00251610(int param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  uint uVar4;
  
  puVar1 = DAT_00261238;
  DAT_00261200 = param_1;
  FUN_00254ff0();
  uVar4 = DAT_00261204 << 0x10 | DAT_00261218;
  if (((char)((uint)puVar1[2] >> 8) < '\0') && (DAT_00261200 != 0)) {
    uVar4 = uVar4 | 1;
  }
  puVar2 = (undefined4 *)*puVar1;
  if ((undefined4 *)puVar1[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x41d7c;
  puVar2[1] = uVar4;
  *puVar1 = puVar2 + 2;
  if ((undefined4 *)puVar1[1] <= puVar2 + 2) {
    FUN_00252040();
  }
  uVar3 = FUN_00250c20();
  *puVar1 = uVar3;
  return;
}


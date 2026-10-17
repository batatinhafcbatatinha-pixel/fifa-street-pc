// callers: 0x19e4a0 0x19e810 0x25d6d0

void FUN_0025ce60(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  FUN_002578d0();
  puVar2 = (undefined4 *)*puVar1;
  if ((undefined4 *)puVar1[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x417fc;
  puVar2[1] = param_1;
  *puVar1 = puVar2 + 2;
  puVar1[2] = puVar1[2] | 0x800;
  return;
}


// callers: 0x198750 0x199840

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_002503d0(uint param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x402a8;
  puVar2[1] = param_1 >> 0x10 & 0xff | (param_1 & 0xff) << 0x10 | param_1 & 0xff00ff00;
  *puVar1 = puVar2 + 2;
  _DAT_002611c8 = param_1;
  return;
}


// callers: 0x198750 0x199da0 0x255450

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_002541d0(float param_1,float param_2)

{
  uint *puVar1;
  uint uVar2;
  
  puVar1 = DAT_00261238;
  DAT_00261238[0x3be] = (uint)(param_1 + _DAT_0025f058);
  puVar1[0x3bf] = (uint)(param_2 + _DAT_0025f058);
  FUN_00254ff0();
  if (puVar1[1] <= *puVar1) {
    FUN_00252040();
  }
  uVar2 = FUN_00250c20();
  *puVar1 = uVar2;
  return;
}


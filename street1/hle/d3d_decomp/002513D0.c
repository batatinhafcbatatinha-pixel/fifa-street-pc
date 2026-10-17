// callers: 0x198750 0x199200

void FUN_002513d0(int param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  puVar1 = DAT_00261238;
  DAT_002611e4 = param_1;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  uVar3 = 0;
  if (DAT_00261228 != 0) {
    uVar3 = 2;
  }
  if ((DAT_00261224 != 0) && ((DAT_002611e0 == 0 || (DAT_002611e4 == 0x1e00)))) {
    uVar3 = uVar3 | 1;
  }
  *puVar2 = 0x41d84;
  puVar2[1] = uVar3;
  puVar2[2] = 0x40370;
  puVar2[3] = param_1;
  *puVar1 = puVar2 + 4;
  return;
}


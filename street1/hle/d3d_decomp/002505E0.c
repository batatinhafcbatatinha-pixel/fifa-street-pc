// callers: 0x252fb0

void FUN_002505e0(uint param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  uint uVar4;
  
  puVar2 = DAT_00261238;
  uVar4 = 0;
  if (((&DAT_0025ed88)[*(byte *)(DAT_00261238[0x681] + 0xd)] & 0x3c) == 0x20) {
    uVar4 = param_1;
  }
  if (uVar4 != (DAT_00261238[2] & 1)) {
    puVar1 = DAT_00261238 + 1;
    DAT_00261238[2] = DAT_00261238[2] ^ 1;
    puVar3 = (undefined4 *)*puVar2;
    if ((undefined4 *)*puVar1 <= puVar3) {
      puVar3 = (undefined4 *)FUN_00252040();
    }
    *puVar3 = 0x40110;
    puVar3[1] = 0;
    puVar3[2] = 0x40100;
    puVar3[3] = uVar4 << 5 | 8;
    *puVar2 = puVar3 + 4;
  }
  DAT_0026121c = param_1;
  return;
}


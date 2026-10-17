// callers: 0x258afd

uint * FUN_00258a84(void)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint *local_8;
  
  uVar2 = FUN_00258a65();
  uVar3 = 0;
  uVar4 = 0;
  puVar1 = &DAT_0025f4e8;
  do {
    local_8 = puVar1;
    if ((*local_8 & 0xff00) == (uVar2 & 0xff00)) break;
    uVar4 = uVar4 + 0xc;
    uVar3 = uVar3 + 1;
    puVar1 = local_8 + 3;
  } while (uVar4 < 0x8ac);
  if (uVar3 < 0xb9) {
    puVar1 = &DAT_0025f4e8 + uVar3 * 3;
    do {
      local_8 = puVar1;
      if ((*local_8 & 0xff) == 0) {
        return local_8;
      }
      if ((*local_8 & 0xff) == (uVar2 & 0xff)) {
        return local_8;
      }
      uVar3 = uVar3 + 1;
      puVar1 = local_8 + 3;
    } while (uVar3 < 0xb9);
  }
  return local_8;
}


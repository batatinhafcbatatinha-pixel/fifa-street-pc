// callers: 0x252fb0 0x255520

void FUN_00257e50(uint *param_1)

{
  uint *puVar1;
  uint uVar2;
  
  uVar2 = *param_1;
  if ((uVar2 & 0x780000) == 0x80000) {
    if ((((uVar2 & 0x70000) == 0x50000) && (puVar1 = (uint *)param_1[5], puVar1 != (uint *)0x0)) &&
       (uVar2 = *puVar1, *puVar1 = uVar2 - 0x80000, (uVar2 - 0x80000 & 0x78ffff) == 0)) {
      FUN_00257b40(puVar1);
    }
    uVar2 = *param_1;
    if ((uVar2 & 0xffff) == 0) {
      FUN_00257b40(param_1);
      return;
    }
  }
  *param_1 = uVar2 - 0x80000;
  return;
}


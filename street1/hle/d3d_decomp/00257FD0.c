// callers: 0x19f0c0

undefined4 * FUN_00257fd0(undefined4 param_1)

{
  undefined4 *puVar1;
  uint uVar2;
  
  puVar1 = (undefined4 *)FUN_001609a7(0xc,0x64800000);
  if (puVar1 != (undefined4 *)0x0) {
    uVar2 = FUN_001609a7(param_1,0xb2800000);
    if (uVar2 != 0) {
      puVar1[1] = uVar2 & 0xfffffff;
      *puVar1 = 0x1000001;
      return puVar1;
    }
    FUN_00160a47(puVar1,0x24800000);
  }
  return (undefined4 *)0x0;
}


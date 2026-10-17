// callers: 0x18e590 0x194d60 0x194e10 0x195320 0x1954e0 0x195c90 0x195db0 0x196480 0x19f010 0x19f110

void thunk_FUN_002520c0(uint *param_1)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  
  if (DAT_00261238 != 0) {
    if ((((*param_1 & 0x70000) == 0x50000) && (puVar1 = (uint *)param_1[5], puVar1 != (uint *)0x0))
       && (iVar3 = FUN_00257be0(param_1), param_1 = puVar1, iVar3 != 0)) {
      FUN_00251d90(*(undefined4 *)(DAT_00261238 + 0x2c),2);
      return;
    }
    uVar2 = param_1[2];
    iVar3 = FUN_00257be0(param_1);
    if (iVar3 != 0) {
      FUN_00251d90(*(undefined4 *)(DAT_00261238 + 0x2c),2);
      return;
    }
    if (uVar2 != 0) {
      FUN_00251d90(uVar2,0x10);
    }
  }
  return;
}


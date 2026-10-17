// callers: 0x18e540 0x194e10 0x195320 0x1954e0 0x195770

void FUN_00257e10(uint *param_1,int param_2)

{
  uint uVar1;
  
  uVar1 = param_1[1] + param_2;
  if ((*param_1 & 0x70000) != 0x20000) {
    uVar1 = uVar1 & 0xfffffff;
  }
  param_1[1] = uVar1;
  return;
}


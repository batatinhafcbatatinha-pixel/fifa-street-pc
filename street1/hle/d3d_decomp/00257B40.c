// callers: 0x253800 0x2539b0 0x254ce0 0x2560c0 0x257ca0 0x257e50

void FUN_00257b40(uint *param_1)

{
  uint uVar1;
  
  uVar1 = *param_1 & 0x70000;
  if ((uVar1 != 0x50000) || ((int)*param_1 < 0)) {
    FUN_002520c0(param_1);
  }
  FUN_00257af0(param_1);
  if (uVar1 == 0x50000) {
    if (-1 < (int)*param_1) goto LAB_00257bc1;
    uVar1 = param_1[1] | 0x80000000;
  }
  else if (uVar1 == 0x20000) {
    if ((int)*param_1 < 0) goto LAB_00257bc1;
    uVar1 = param_1[1];
  }
  else {
    if ((uVar1 == 0x10000) || (uVar1 == 0x60000)) goto LAB_00257bc1;
    uVar1 = param_1[1] | 0x80000000;
  }
  FUN_00160a47(uVar1,0xa1800000);
LAB_00257bc1:
  FUN_00160a47(param_1,0x24800000);
  return;
}


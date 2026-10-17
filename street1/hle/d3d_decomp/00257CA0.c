// callers: 0x1958d0 0x1970c0 0x1995d0 0x199c10 0x19a210 0x19a840 0x19a900 0x19ad80 0x19f010 0x19f110 0x19f290 0x255520 0x257ca0 0x25d660 0x25e4a0 0x25e820

uint FUN_00257ca0(uint *param_1)

{
  uint uVar1;
  
  uVar1 = *param_1;
  if ((uVar1 & 0xffff) == 1) {
    if (((uVar1 & 0x70000) == 0x50000) && (param_1[5] != 0)) {
      FUN_00257ca0(param_1[5]);
    }
    uVar1 = *param_1;
    if ((uVar1 & 0x780000) == 0) {
      FUN_00257b40(param_1);
      return 0;
    }
  }
  *param_1 = uVar1 - 1;
  return uVar1 - 1 & 0xffff;
}


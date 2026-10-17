// callers: 0x253230 0x2535b0 0x255650 0x257c60 0x25a7d0 0x25d5d0

uint FUN_00257c60(uint *param_1)

{
  uint uVar1;
  
  if ((((*param_1 & 0xffff) == 0) && ((*param_1 & 0x70000) == 0x50000)) && (param_1[5] != 0)) {
    FUN_00257c60(param_1[5]);
  }
  uVar1 = *param_1;
  *param_1 = uVar1 + 1;
  return uVar1 + 1 & 0xffff;
}


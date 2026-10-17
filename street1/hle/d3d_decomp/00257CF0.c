// callers: 0x2584e0

char FUN_00257cf0(uint *param_1)

{
  uint uVar1;
  
  uVar1 = *param_1 & 0x70000;
  if (0x30000 < uVar1) {
    if (uVar1 == 0x40000) {
      if ((param_1[3] & 4) != 0) {
        return '\x05';
      }
      return (0x20 < (param_1[3] & 0xf0)) + '\x03';
    }
    if (uVar1 != 0x50000) {
      return '\n';
    }
    return (0x20 < (param_1[3] & 0xf0)) + '\x01';
  }
  if (uVar1 == 0x30000) {
    return '\t';
  }
  if (uVar1 != 0) {
    return (uVar1 != 0x10000) + '\a';
  }
  return '\x06';
}


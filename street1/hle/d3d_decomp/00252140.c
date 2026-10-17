// callers: 0x258020 0x258070

void FUN_00252140(uint *param_1)

{
  if (DAT_00261238 != 0) {
    if ((*param_1 & 0x780000) != 0) {
      FUN_00251d90(*(undefined4 *)(DAT_00261238 + 0x2c),2);
      return;
    }
    if (param_1[2] != 0) {
      FUN_00251d90(param_1[2],0x10);
    }
  }
  return;
}


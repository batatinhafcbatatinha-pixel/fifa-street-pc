// callers: 0x2520c0

undefined4 FUN_00257be0(uint *param_1)

{
  if (((*param_1 & 0x780000) == 0) &&
     ((((*param_1 & 0x70000) != 0x50000 || ((uint *)param_1[5] == (uint *)0x0)) ||
      ((*(uint *)param_1[5] & 0x780000) == 0)))) {
    return 0;
  }
  return 1;
}


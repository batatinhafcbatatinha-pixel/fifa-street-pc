// callers: 0x252fb0

void FUN_00257c20(uint *param_1)

{
  int *piVar1;
  
  if ((((*param_1 & 0x780000) == 0) && ((*param_1 & 0x70000) == 0x50000)) &&
     (piVar1 = (int *)param_1[5], piVar1 != (int *)0x0)) {
    *piVar1 = *piVar1 + 0x80000;
  }
  *param_1 = *param_1 + 0x80000;
  return;
}


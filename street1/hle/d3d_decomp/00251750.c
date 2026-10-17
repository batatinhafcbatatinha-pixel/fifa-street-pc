// callers: 0x251d90

uint FUN_00251750(uint param_1)

{
  uint uVar1;
  uint *in_ECX;
  int in_EDX;
  
  if (param_1 <= *in_ECX) {
    param_1 = in_ECX[0x11] + param_1;
  }
  uVar1 = *(uint *)(in_EDX + 4);
  if (uVar1 <= *in_ECX) {
    uVar1 = uVar1 + in_ECX[0x11];
  }
  return uVar1 - param_1 & ((int)(uVar1 - param_1) < 0) - 1;
}


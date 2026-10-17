// callers: 0x259ccd

void FUN_0025bb2b(int param_1,undefined4 param_2,uint param_3,uint param_4,uint param_5)

{
  undefined4 *puVar1;
  int *in_ECX;
  
  puVar1 = (undefined4 *)(*in_ECX + in_ECX[0x4c] + param_1 * 8);
  *puVar1 = param_2;
  puVar1[1] = ((param_3 | 0xffffff80) << 8 | param_5) << 0x10 | param_4 & 0xffff;
  return;
}


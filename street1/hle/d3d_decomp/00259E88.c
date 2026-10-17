// callers: 0x259ee2

void FUN_00259e88(uint param_1,uint *param_2,undefined4 *param_3,int param_4)

{
  if (param_4 == 0) {
    if ((param_1 & 0xf0000000) == 0x80000000) {
      *param_2 = param_1 & 0xfffffff;
      *param_3 = 1;
    }
    else {
      *param_2 = param_1;
      *param_3 = 2;
    }
  }
  else {
    *param_2 = param_1 & 0xfffffff | 0x40000000;
    *param_3 = 3;
  }
  return;
}


// callers: 0x259190 0x2592b0

void FUN_00257eb2(int param_1)

{
  int *in_ECX;
  
  if (in_ECX[0x6e] == 0) {
    if ((((in_ECX[0x6d] & 0x1000000U) == 0) || ((in_ECX[2] & 0xc0000000U) == 0x80000000)) ||
       (in_ECX[0x74] != 0)) {
      if ((((in_ECX[2] & 0xc0000000U) == 0x80000000) && ((in_ECX[0x6d] & 0x200000U) != 0)) &&
         (in_ECX[0x74] == 0)) {
        param_1 = param_1 - in_ECX[1];
      }
    }
    else {
      param_1 = param_1 - ((uint)in_ECX[1] >> 1);
    }
    *(int *)(*in_ECX + 0x600800) = param_1;
  }
  return;
}


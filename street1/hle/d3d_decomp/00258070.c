// callers: 

uint FUN_00258070(int param_1,byte param_2)

{
  if ((param_2 & 0xa0) == 0) {
    FUN_00252140(param_1);
  }
  return *(uint *)(param_1 + 4) | 0x80000000;
}


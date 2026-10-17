
void FUN_00194d60(void)

{
  int *in_ECX;
  
  if (in_ECX[0xe] != 0) {
    if ((&DAT_002f3470)[*in_ECX] == in_ECX[0xe]) {
      FUN_002539b0(*in_ECX,0);
      (&DAT_002f3470)[*in_ECX] = 0;
    }
    thunk_FUN_002520c0(in_ECX[0xe]);
    (*(code *)PTR_FUN_002d4d3c)(in_ECX[0xe],0xc);
    in_ECX[0xe] = 0;
  }
  return;
}


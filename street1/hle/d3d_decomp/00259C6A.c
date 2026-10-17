// callers: 0x255bf0

undefined4
FUN_00259c6a(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            int *param_5)

{
  int iVar1;
  int *in_ECX;
  
  in_ECX[0x40] = 0;
  in_ECX[0x41] = 0;
  in_ECX[0x44] = 0;
  in_ECX[0x46] = 0;
  iVar1 = in_ECX[0x58];
  in_ECX[0x43] = iVar1;
  in_ECX[0x58] = iVar1 + 0x37f;
  FUN_0025b99e(0x80,0x80,8,param_3);
  *param_5 = (in_ECX[0x40] + 0x80) * 0x10000 + *in_ECX;
  return 1;
}


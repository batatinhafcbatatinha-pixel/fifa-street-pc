// callers: 0x255bf0 0x259fb1

undefined4 FUN_00259ccd(uint *param_1)

{
  uint uVar1;
  int in_ECX;
  
  uVar1 = *param_1;
  FUN_0025bb2b(((uVar1 >> 0xb ^ uVar1) >> 0xb ^ uVar1) & 0x7ff,uVar1,*(undefined4 *)(in_ECX + 0x100)
               ,param_1[3],*(undefined2 *)((int)param_1 + 6));
  return 1;
}


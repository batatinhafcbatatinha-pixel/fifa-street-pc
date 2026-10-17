// callers: 0x198750

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0025d080(void)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint in_stack_00000010;
  undefined4 in_stack_00000014;
  undefined4 *in_stack_00000018;
  
  if (DAT_002639ec == 0) {
    DAT_002639ec = 0x80000;
  }
  if (DAT_002639e8 == 0) {
    DAT_002639e8 = 0x8000;
  }
  DAT_00261248 = DAT_00261248 | in_stack_00000010 & 0x10;
  DAT_00261238 = &DAT_00261240;
  _DAT_00261b78 = 1;
  iVar1 = FUN_00255bf0(in_stack_00000014);
  if (iVar1 < 0) {
    FUN_002560c0();
    if (in_stack_00000018 != (undefined4 *)0x0) {
      *in_stack_00000018 = 0;
    }
    puVar3 = &DAT_00261240;
    for (iVar2 = 0x928; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = 0;
      puVar3 = puVar3 + 1;
    }
    DAT_00261238 = (undefined4 *)0x0;
    return iVar1;
  }
  if (in_stack_00000018 != (undefined4 *)0x0) {
    *in_stack_00000018 = &DAT_00261240;
  }
  return 0;
}


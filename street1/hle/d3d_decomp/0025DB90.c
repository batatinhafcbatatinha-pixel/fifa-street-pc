// callers: 0x198750

undefined4 FUN_0025db90(uint param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = DAT_00261238;
  if ((*(int *)(DAT_00261238 + 0x8d4) != 0) || (*(int *)(DAT_00261238 + 0x8d8) != 0)) {
    FUN_0025e3f0();
  }
  if (param_1 == 0) {
    param_1 = 5;
  }
  if ((*(uint *)(iVar2 + 8) & 0x4000) == 0) {
    if ((param_1 & 4) == 0) goto LAB_0025dc42;
    FUN_00251cd0(0);
    iVar1 = *(int *)(iVar2 + 0x1974 + (*(int *)(iVar2 + 0x2478) - 1U & 1) * 4);
    if (iVar1 != 0) {
      FUN_00251d90(iVar1,1);
    }
    *(int *)(iVar2 + 0x2478) = *(int *)(iVar2 + 0x2478) + 1;
    FUN_0025da40();
  }
  else {
    if ((param_1 & 3) != 0) {
      FUN_00251cd0(0);
      iVar1 = *(int *)(iVar2 + 0x1974 + (*(int *)(iVar2 + 0x2478) - 1U & 1) * 4);
      if (iVar1 != 0) {
        FUN_00251d90(iVar1,1);
      }
      *(int *)(iVar2 + 0x2478) = *(int *)(iVar2 + 0x2478) + 1;
    }
    FUN_0025d850(param_1);
    if ((param_1 & 4) == 0) goto LAB_0025dc42;
  }
  FUN_0025db50();
LAB_0025dc42:
  if (((param_1 & 4) != 0) &&
     ((DAT_00260d98 = DAT_00260d98 | 0xff1000, *(int *)(iVar2 + 0x8d4) != 0 ||
      (*(int *)(iVar2 + 0x8d8) != 0)))) {
    FUN_0025e820();
  }
  return *(undefined4 *)(iVar2 + 0x2478);
}


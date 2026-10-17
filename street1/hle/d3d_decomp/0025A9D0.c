// callers: 0x2578d0

undefined4 FUN_0025a9d0(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x784) != 0) {
    return param_2;
  }
  *(uint *)(param_1 + 8) = *(uint *)(param_1 + 8) & 0xfffeffbf;
  iVar1 = 0;
  if (DAT_00261178 != 0) {
    iVar1 = 3;
  }
                    /* WARNING: Could not recover jumptable at 0x0025aa87. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar2 = (*(code *)(&PTR_LAB_0025af28)[(&DAT_00260dd0)[iVar1 * 0x20] + -1])();
  return uVar2;
}


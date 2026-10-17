// callers: 0x253d10 0x255520 0x255650

void FUN_00253c30(int param_1,int *param_2)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  
  if ((param_2 != (int *)0x0) && (uVar2 = param_2[1], uVar2 != 0)) {
    iVar3 = param_2[2];
    iVar4 = *param_2;
    iVar5 = param_2[3];
    iVar7 = param_2[4];
    iVar6 = param_2[5];
    if (-1 < iVar4) {
      iVar6 = 0;
      iVar7 = 0;
    }
    piVar1 = (int *)(DAT_00261238 + 0x1ac0 + param_1 * 0x18);
    *piVar1 = iVar4;
    piVar1[1] = uVar2;
    piVar1[2] = iVar3;
    piVar1[3] = iVar5;
    piVar1[4] = iVar7;
    piVar1[5] = iVar6;
    FUN_0025a356(param_1,uVar2 & 0xfffffff,iVar3,iVar5,iVar4,iVar7,iVar6);
    return;
  }
  iVar3 = DAT_00261238 + param_1 * 0x18;
  FUN_0025a544(param_1,*(undefined4 *)(DAT_00261238 + 0x1ad4 + param_1 * 0x18));
  *(undefined4 *)(iVar3 + 0x1ac0) = 0;
  *(undefined4 *)(iVar3 + 0x1ac4) = 0;
  *(undefined4 *)(iVar3 + 0x1ac8) = 0;
  *(undefined4 *)(iVar3 + 0x1acc) = 0;
  *(undefined4 *)(iVar3 + 0x1ad0) = 0;
  *(undefined4 *)(iVar3 + 0x1ad4) = 0;
  return;
}


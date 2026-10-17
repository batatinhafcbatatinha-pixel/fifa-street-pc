// callers: 0x254970 0x254f90 0x254ff0

void FUN_00254660(int *param_1,int *param_2,uint param_3)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int *piVar4;
  int *piVar5;
  
  do {
    uVar3 = param_3;
    if (0x1f < param_3) {
      uVar3 = 0x20;
    }
    *param_1 = uVar3 * 0x40000 + 0xb00;
    piVar4 = param_2;
    piVar5 = param_1;
    for (uVar1 = uVar3 & 0x3fffffff; piVar5 = piVar5 + 1, uVar1 != 0; uVar1 = uVar1 - 1) {
      *piVar5 = *piVar4;
      piVar4 = piVar4 + 1;
    }
    for (iVar2 = 0; iVar2 != 0; iVar2 = iVar2 + -1) {
      *(char *)piVar5 = (char)*piVar4;
      piVar4 = (int *)((int)piVar4 + 1);
      piVar5 = (int *)((int)piVar5 + 1);
    }
    param_2 = param_2 + uVar3;
    param_3 = param_3 - uVar3;
    param_1 = param_1 + uVar3 + 1;
  } while (param_3 != 0);
  return;
}


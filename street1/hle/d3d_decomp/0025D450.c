// callers: 0x25d850 0x25dd90

void FUN_0025d450(int *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  int *piVar7;
  
  puVar2 = DAT_00261238;
  FUN_0025dec0(*param_1);
  uVar6 = puVar2[2];
  if (param_1[2] != DAT_00260e50) {
    FUN_00250ac0(1,0xc,param_1[2]);
  }
  piVar7 = param_1 + 4;
  uVar5 = 0;
  do {
    iVar1 = *(int *)((int)&DAT_0025ffe8 + uVar5);
    if (((0x5b < iVar1) || ((*(byte *)(DAT_00261238 + 2) & 0x10) == 0)) &&
       (*piVar7 != (&DAT_00260fa0)[iVar1])) {
      FUN_00250080(iVar1,*piVar7);
    }
    uVar5 = uVar5 + 8;
    piVar7 = piVar7 + 1;
  } while (uVar5 < 0xa0);
  piVar7 = param_1 + 0x18;
  uVar5 = 0;
  do {
    if (*piVar7 != (&DAT_00260da0)[*(int *)((int)&DAT_0025ff90 + uVar5)]) {
      FUN_00250ac0(0,*(int *)((int)&DAT_0025ff90 + uVar5),*piVar7);
    }
    uVar5 = uVar5 + 8;
    piVar7 = piVar7 + 1;
  } while (uVar5 < 0x58);
  if ((uVar6 & 0x10) == 0) {
    if (*param_1 != 0) {
      uVar6 = 0;
      piVar7 = param_1 + 0x23;
      do {
        if (((*(int *)(*(int *)(*param_1 + 8) + 0x20) != 0) ||
            (*(int *)(*(int *)(*param_1 + 8) + 0x24) != 0)) || ((uVar6 < 8 || (9 < uVar6)))) {
          FUN_00250080(uVar6,*piVar7);
        }
        uVar6 = uVar6 + 1;
        piVar7 = piVar7 + 1;
      } while (uVar6 < 0x39);
      FUN_00250080(0x88,param_1[3]);
    }
    FUN_00254f90(puVar2 + 0x42a,8);
  }
  puVar3 = (undefined4 *)*puVar2;
  if ((undefined4 *)puVar2[1] <= puVar3) {
    puVar3 = (undefined4 *)FUN_00252040();
  }
  uVar6 = *(uint *)(puVar2[0x1e5] + 4);
  if ((uVar6 & 0x12) != 0) {
    *puVar3 = 0x81e94;
    puVar3[1] = 6;
    puVar3[2] = uVar6 & 1;
    uVar4 = puVar2[0x1e7];
    puVar3[3] = 0x41ea0;
    puVar3[4] = uVar4;
    uVar4 = FUN_00250c20();
    *puVar2 = uVar4;
    return;
  }
  *puVar3 = 0x41e94;
  puVar3[1] = 4;
  uVar4 = FUN_00250c20();
  *puVar2 = uVar4;
  return;
}


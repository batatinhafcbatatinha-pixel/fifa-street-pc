// callers: 0x25d850 0x25dd90

void FUN_0025d330(void)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  uint uVar4;
  
  puVar2 = DAT_00261238;
  uVar4 = 0;
  do {
    iVar1 = *(int *)((int)&DAT_0025ffe8 + uVar4);
    if (((iVar1 < 0x5c) && ((*(byte *)(DAT_00261238 + 2) & 0x10) != 0)) ||
       (*(int *)((int)&DAT_0025ffec + uVar4) != (&DAT_00260fa0)[iVar1])) {
      FUN_00250080(iVar1,*(int *)((int)&DAT_0025ffec + uVar4));
    }
    uVar4 = uVar4 + 8;
  } while (uVar4 < 0xa0);
  uVar4 = 0;
  do {
    if (*(int *)((int)&DAT_0025ff94 + uVar4) !=
        (&DAT_00260da0)[*(int *)((int)&DAT_0025ff90 + uVar4)]) {
      FUN_00250ac0(0,*(int *)((int)&DAT_0025ff90 + uVar4),*(int *)((int)&DAT_0025ff94 + uVar4));
    }
    uVar4 = uVar4 + 8;
  } while (uVar4 < 0x58);
  FUN_0025dec0(0);
  iVar1 = DAT_00261198;
  if (DAT_00261198 != DAT_00260db0) {
    FUN_00250ac0(0,4,DAT_00261198);
  }
  if (iVar1 != DAT_00260dac) {
    FUN_00250ac0(0,3,iVar1);
  }
  if (DAT_00260e50 != 1) {
    FUN_00250ac0(1,0xc,1);
  }
  FUN_002578d0();
  FUN_00254f90(&DAT_00260088,8);
  puVar3 = (undefined4 *)*puVar2;
  if ((undefined4 *)puVar2[1] <= puVar3) {
    puVar3 = (undefined4 *)FUN_00252040();
  }
  *puVar3 = 0x81e94;
  puVar3[1] = 6;
  puVar3[2] = 0;
  puVar3[3] = 0x41ea0;
  puVar3[4] = 0;
  puVar3[5] = &LAB_00080394;
  puVar3[6] = 0;
  puVar3[7] = 0x4b7fffff;
  *puVar2 = puVar3 + 8;
  return;
}


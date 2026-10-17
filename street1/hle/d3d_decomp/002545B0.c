// callers: 0x254d70

void FUN_002545b0(uint *param_1,uint *param_2)

{
  uint *puVar1;
  uint *puVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint *puVar7;
  uint *puVar8;
  
  puVar1 = param_1 + 0x45;
  param_2 = (uint *)((int)param_2 * 0x10 + 0x10a8 + DAT_00261238);
  do {
    uVar5 = *puVar1 >> 0x12;
    puVar2 = puVar1 + 1;
    if (uVar5 == 1) {
      param_1 = (uint *)(&DAT_00260180 + *puVar2 * 0x10);
    }
    else {
      uVar4 = uVar5;
      puVar6 = puVar2;
      puVar8 = param_1;
      puVar7 = param_2;
      if ((*puVar1 & 0xffff) == 0xb00) {
        for (; uVar4 != 0; uVar4 = uVar4 - 1) {
          *puVar7 = *puVar6;
          puVar6 = puVar6 + 1;
          puVar7 = puVar7 + 1;
        }
        for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
          *(char *)puVar7 = (char)*puVar6;
          puVar6 = (uint *)((int)puVar6 + 1);
          puVar7 = (uint *)((int)puVar7 + 1);
        }
        param_2 = param_2 + uVar5;
      }
      else {
        for (; uVar4 != 0; uVar4 = uVar4 - 1) {
          *puVar8 = *puVar6;
          puVar6 = puVar6 + 1;
          puVar8 = puVar8 + 1;
        }
        for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
          *(char *)puVar8 = (char)*puVar6;
          puVar6 = (uint *)((int)puVar6 + 1);
          puVar8 = (uint *)((int)puVar8 + 1);
        }
        param_1 = param_1 + uVar5;
      }
    }
    puVar1 = puVar2 + uVar5;
  } while (puVar2[uVar5] != 0);
  return;
}


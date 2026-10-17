// callers: 0x259690 0x25b070

void FUN_0025afd0(uint *param_1)

{
  uint uVar1;
  uint uVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint *puVar7;
  
  do {
    uVar1 = *param_1;
    *(uint *)param_1[2] = (uVar1 & 0xfffffff) + 1;
    if ((uVar1 & 0x20000000) != 0) {
      uVar2 = param_1[4];
      puVar3 = (uint *)param_1[3];
      uVar5 = *puVar3;
      while (uVar5 != 0xffffffff) {
        puVar6 = puVar3 + 2;
        puVar7 = (uint *)(puVar3[1] + uVar2);
        for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
          *puVar7 = *puVar6;
          puVar6 = puVar6 + 1;
          puVar7 = puVar7 + 1;
        }
        puVar3 = (uint *)((int)(puVar3 + 2) + uVar5);
        for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
          *(char *)puVar7 = (char)*puVar6;
          puVar6 = (uint *)((int)puVar6 + 1);
          puVar7 = (uint *)((int)puVar7 + 1);
        }
        uVar5 = *puVar3;
      }
    }
    puVar3 = (uint *)param_1[1];
    param_1[1] = 0xffffffff;
    if ((int)uVar1 < 0) {
      *param_1 = uVar1 & 0xbfffffff;
    }
    param_1 = puVar3;
  } while (1 < (int)puVar3 + 1U);
  return;
}


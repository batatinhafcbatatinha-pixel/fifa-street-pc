// callers: 0x25b070

void FUN_0025c9e0(undefined8 *param_1,undefined8 *param_2,int param_3)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  uint uVar8;
  
  uVar8 = (-(int)param_1 & 0x1fU) >> 2;
  if ((int)uVar8 < param_3) {
    param_3 = param_3 - uVar8;
    for (; uVar8 != 0; uVar8 = uVar8 - 1) {
      *(undefined4 *)param_1 = *(undefined4 *)param_2;
      param_2 = (undefined8 *)((int)param_2 + 4);
      param_1 = (undefined8 *)((int)param_1 + 4);
    }
    while (0xf < param_3) {
      uVar1 = param_2[1];
      uVar2 = param_2[2];
      uVar3 = param_2[3];
      uVar4 = param_2[4];
      uVar5 = param_2[5];
      uVar6 = param_2[6];
      uVar7 = param_2[7];
      *param_1 = *param_2;
      param_1[1] = uVar1;
      param_1[2] = uVar2;
      param_1[3] = uVar3;
      param_1[4] = uVar4;
      param_1[5] = uVar5;
      param_1[6] = uVar6;
      param_1[7] = uVar7;
      param_2 = param_2 + 8;
      param_1 = param_1 + 8;
      param_3 = param_3 + -0x10;
    }
  }
  for (; param_3 != 0; param_3 = param_3 + -1) {
    *(undefined4 *)param_1 = *(undefined4 *)param_2;
    param_2 = (undefined8 *)((int)param_2 + 4);
    param_1 = (undefined8 *)((int)param_1 + 4);
  }
  return;
}


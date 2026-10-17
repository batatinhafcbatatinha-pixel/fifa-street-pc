// callers: 0x255450

void FUN_00252af0(int param_1,undefined4 *param_2)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  
  iVar1 = DAT_00261238;
  puVar4 = param_2;
  puVar5 = (undefined4 *)(param_1 * 0x40 + 0xc60 + DAT_00261238);
  for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar5 = *puVar4;
    puVar4 = puVar4 + 1;
    puVar5 = puVar5 + 1;
  }
  DAT_00260d98 = DAT_00260d98 | *(uint *)(&DAT_0025f06c + param_1 * 4);
  if (param_1 == 1) {
    if (((float)param_2[10] == (float)param_2[0xb]) || ((float)param_2[10] == 0.0)) {
      *(undefined4 *)(iVar1 + 0x93c) = 0;
      *(undefined4 *)(iVar1 + 0x940) = 0x3f800000;
    }
    else {
      *(float *)(iVar1 + 0x93c) =
           (float)param_2[0xf] - ((float)param_2[0xe] / (float)param_2[10]) * (float)param_2[0xb];
      *(float *)(iVar1 + 0x940) =
           (((float)param_2[0xf] - (float)param_2[0xe]) / ((float)param_2[10] - (float)param_2[0xb])
           ) * (float)param_2[0xb] + (float)param_2[0xf];
    }
    uVar2 = *(uint *)(iVar1 + 8) & 0xfffffffd;
    *(uint *)(iVar1 + 8) = uVar2;
    *(float *)(iVar1 + 0x944) = 1.0 / *(float *)(iVar1 + 0x940);
    if (((((float)param_2[3] == 0.0) && ((float)param_2[7] == 0.0)) && ((float)param_2[0xb] == 0.0))
       && (param_2[0xf] == 0x3f800000)) {
      *(uint *)(iVar1 + 8) = uVar2 | 2;
    }
    FUN_00250210();
    FUN_00254ff0();
  }
  return;
}


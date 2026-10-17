// callers: 0x250c20

void FUN_002500e0(float *param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  
  fVar6 = (float)*(int *)(DAT_00261238 + 0xee0);
  if (*(int *)(DAT_00261238 + 0xee0) < 0) {
    fVar6 = fVar6 + 4.2949673e+09;
  }
  fVar6 = fVar6 * *(float *)(DAT_00261238 + 0x95c) + *(float *)(DAT_00261238 + 0xef8);
  fVar7 = (float)*(int *)(DAT_00261238 + 0xee4);
  if (*(int *)(DAT_00261238 + 0xee4) < 0) {
    fVar7 = fVar7 + 4.2949673e+09;
  }
  fVar7 = fVar7 * *(float *)(DAT_00261238 + 0x960) + *(float *)(DAT_00261238 + 0xefc);
  if (((char)((uint)*(undefined4 *)(DAT_00261238 + 8) >> 8) < '\0') && (DAT_00261200 != 0)) {
    fVar6 = fVar6 - 0.5;
    fVar7 = fVar7 - 0.5;
  }
  fVar8 = (float)*(int *)(DAT_00261238 + 0xee8);
  if (*(int *)(DAT_00261238 + 0xee8) < 0) {
    fVar8 = fVar8 + 4.2949673e+09;
  }
  fVar8 = fVar8 * *(float *)(DAT_00261238 + 0x95c) * 0.5;
  fVar1 = (float)*(int *)(DAT_00261238 + 0xeec);
  if (*(int *)(DAT_00261238 + 0xeec) < 0) {
    fVar1 = fVar1 + 4.2949673e+09;
  }
  fVar9 = fVar1 * *(float *)(DAT_00261238 + 0x960) * -0.5;
  fVar1 = *(float *)(DAT_00261238 + 0xef4);
  fVar2 = *(float *)(DAT_00261238 + 0xef0);
  fVar3 = *(float *)(DAT_00261238 + 0x948);
  fVar4 = *(float *)(DAT_00261238 + 0xef0);
  fVar5 = *(float *)(DAT_00261238 + 0x948);
  param_1[3] = 0.0;
  param_1[2] = fVar4 * fVar5;
  *param_1 = fVar8 + fVar6;
  param_1[1] = -fVar9 + fVar7;
  param_2[1] = fVar9;
  param_2[2] = (fVar1 - fVar2) * fVar3;
  param_2[3] = 0.0;
  *param_2 = fVar8;
  return;
}


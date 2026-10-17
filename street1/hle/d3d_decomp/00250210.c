// callers: 0x2512a0 0x252af0 0x2535d0

void FUN_00250210(void)

{
  float fVar1;
  int iVar2;
  float *pfVar3;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50 [5];
  float local_3c;
  float local_28;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  
  fVar1 = (float)*(int *)(DAT_00261238 + 0xee8);
  if (*(int *)(DAT_00261238 + 0xee8) < 0) {
    fVar1 = fVar1 + 4.2949673e+09;
  }
  local_5c = fVar1 * *(float *)(DAT_00261238 + 0x95c) * 0.5;
  fVar1 = (float)*(int *)(DAT_00261238 + 0xeec);
  if (*(int *)(DAT_00261238 + 0xeec) < 0) {
    fVar1 = fVar1 + 4.2949673e+09;
  }
  local_3c = fVar1 * *(float *)(DAT_00261238 + 0x960) * -0.5;
  local_58 = -local_3c;
  local_60 = (*(float *)(DAT_00261238 + 0xef4) - *(float *)(DAT_00261238 + 0xef0)) *
             *(float *)(DAT_00261238 + 0x948);
  local_54 = *(float *)(DAT_00261238 + 0x948) * *(float *)(DAT_00261238 + 0xef0);
  if (DAT_002611dc == 2) {
    local_14 = *(float *)(DAT_00261238 + 0x944) * *(float *)(DAT_00261238 + 0x948);
    local_3c = local_3c * local_14;
    local_60 = local_14 * local_60;
    local_5c = local_14 * local_5c;
    local_58 = local_14 * local_58;
    local_54 = local_14 * local_54;
  }
  else {
    local_14 = 1.0;
  }
  pfVar3 = local_50;
  for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
    *pfVar3 = 0.0;
    pfVar3 = pfVar3 + 1;
  }
  local_28 = local_60;
  local_20 = local_5c;
  local_1c = local_58;
  local_18 = local_54;
  local_50[0] = local_5c;
  FUN_002521e0(DAT_00261238 + 0x980,DAT_00261238 + 0xca0,local_50);
  DAT_00260d98 = DAT_00260d98 | 0x200;
  return;
}


// callers: 0x257240

void FUN_002522e0(float param_1,float *param_2,float *param_3)

{
  float fVar1;
  int iVar2;
  float10 fVar3;
  
  if (param_1 < 1.0) {
    if (param_1 == 0.0) {
      fVar3 = (float10)0.0;
    }
    else {
      fVar3 = (float10)FUN_002529c0(-4.158883 / param_1);
    }
    *param_2 = (float)-fVar3;
    *param_3 = (float)((float10)1.0 - ((float10)1.0 - fVar3) * (float10)param_1);
    return;
  }
  fVar3 = (float10)FUN_00252a20(param_1);
  iVar2 = FUN_002529b0((float)(fVar3 * (float10)4.328085));
  fVar1 = (float)(fVar3 * (float10)4.328085) - (float)iVar2;
  *param_2 = fVar1 * *(float *)(&DAT_0025efc4 + iVar2 * 4) +
             (1.0 - fVar1) * *(float *)(&DAT_0025efc0 + iVar2 * 4);
  *param_3 = fVar1 * *(float *)(&DAT_0025ef44 + iVar2 * 4) +
             (1.0 - fVar1) * *(float *)(&DAT_0025ef40 + iVar2 * 4);
  return;
}


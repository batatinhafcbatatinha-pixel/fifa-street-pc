// callers: 0x257380

void FUN_00252180(undefined8 *param_1,float *param_2,float param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  
  fVar1 = *param_2;
  fVar4 = param_4[2];
  fVar2 = param_2[1];
  fVar5 = param_4[6];
  fVar3 = param_2[2];
  fVar6 = param_4[10];
  fVar7 = param_4[0xe];
  *param_1 = CONCAT44(fVar1 * param_4[1] + fVar2 * param_4[5] + fVar3 * param_4[9] +
                      param_3 * param_4[0xd],
                      fVar1 * *param_4 + fVar2 * param_4[4] + fVar3 * param_4[8] +
                      param_3 * param_4[0xc]);
  *(float *)(param_1 + 1) = fVar1 * fVar4 + fVar2 * fVar5 + fVar3 * fVar6 + param_3 * fVar7;
  return;
}


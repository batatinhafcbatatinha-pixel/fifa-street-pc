// callers: 0x257380

void FUN_00252410(float *param_1)

{
  float *extraout_EDX;
  float10 fVar1;
  
  fVar1 = (float10)FUN_002523b0(param_1[2] * param_1[2] +
                                param_1[1] * param_1[1] + *param_1 * *param_1);
  *extraout_EDX = (float)(fVar1 * (float10)*extraout_EDX);
  extraout_EDX[1] = (float)(fVar1 * (float10)extraout_EDX[1]);
  extraout_EDX[2] = (float)(fVar1 * (float10)extraout_EDX[2]);
  return;
}


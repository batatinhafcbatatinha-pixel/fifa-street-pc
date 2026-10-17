// callers: 0x252410 0x252460 0x257380

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_002523b0(float param_1)

{
  float fVar1;
  
  fVar1 = (float)(-0x41800000 - (int)param_1 >> 1);
  fVar1 = ABS(fVar1 * (_DAT_002ceab4 - _DAT_002ceab0 * param_1 * fVar1 * fVar1));
  return ((float10)3.0 - (float10)fVar1 * (float10)fVar1 * (float10)param_1) * (float10)fVar1 *
         (float10)0.5;
}


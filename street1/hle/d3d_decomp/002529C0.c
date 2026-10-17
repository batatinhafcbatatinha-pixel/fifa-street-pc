// callers: 0x2522e0

/* WARNING: Removing unreachable block (ram,0x00252a06) */

float10 FUN_002529c0(float param_1)

{
  float10 fVar1;
  float10 fVar2;
  float10 fVar3;
  
  fVar1 = ROUND((float10)1.4426950408889634 * (float10)param_1);
  fVar2 = (float10)1.4426950408889634 * (float10)param_1 - fVar1;
  fVar3 = (float10)f2xm1(ABS(fVar2));
  fVar3 = (float10)1 + fVar3;
  if (fVar2 < (float10)0) {
    fVar3 = (float10)1 / fVar3;
  }
  if (fVar1 != (float10)0) {
    fVar3 = (float10)fscale(fVar3,fVar1);
  }
  return fVar3;
}


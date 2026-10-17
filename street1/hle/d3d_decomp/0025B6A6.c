// callers: 0x259d6d

void FUN_0025b6a6(void)

{
  int iVar1;
  int *in_ECX;
  
  iVar1 = *in_ECX;
  in_ECX[0x48] = 0xff;
  in_ECX[0x49] = 0x800000;
  in_ECX[0x47] = 0x1111111;
  *(undefined4 *)(iVar1 + 0x3210) = 0;
  *(undefined4 *)(iVar1 + 0x3270) = 0;
  *(undefined4 *)(iVar1 + 0x3240) = 0;
  *(undefined4 *)(iVar1 + 0x3244) = 0;
  *(undefined4 *)(iVar1 + 0x3058) = 0;
  *(undefined4 *)(iVar1 + 0x3258) = 0;
  *(int *)(iVar1 + 0x2504) = in_ECX[0x41];
  *(undefined4 *)(iVar1 + 0x2508) = 0;
  *(undefined4 *)(iVar1 + 0x250c) = 0;
  *(undefined4 *)(iVar1 + 0x3228) = 0;
  *(undefined4 *)(iVar1 + 0x2410) = 0;
  *(undefined4 *)(iVar1 + 0x2420) = 0;
  return;
}


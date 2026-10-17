// callers: 0x25d850 0x25dd90

void FUN_0025d130(void)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  int *in_EDX;
  
  iVar1 = DAT_00261238;
  *in_EDX = *(int *)(DAT_00261238 + 0x784);
  in_EDX[1] = *(int *)(iVar1 + 0x798);
  in_EDX[2] = DAT_00260e50;
  uVar2 = 0;
  piVar3 = in_EDX;
  do {
    piVar3[4] = (&DAT_00260fa0)[*(int *)((int)&DAT_0025ffe8 + uVar2)];
    piVar3[5] = (&DAT_00260fa0)[*(int *)((int)&DAT_0025fff0 + uVar2)];
    piVar3[6] = (&DAT_00260fa0)[*(int *)((int)&DAT_0025fff8 + uVar2)];
    piVar3[7] = (&DAT_00260fa0)[*(int *)((int)&DAT_00260000 + uVar2)];
    piVar3[8] = (&DAT_00260fa0)[*(int *)((int)&DAT_00260008 + uVar2)];
    uVar2 = uVar2 + 0x28;
    piVar3 = piVar3 + 5;
  } while (uVar2 < 0xa0);
  piVar3 = in_EDX + 0x18;
  uVar2 = 0;
  do {
    *piVar3 = (&DAT_00260da0)[*(int *)((int)&DAT_0025ff90 + uVar2)];
    uVar2 = uVar2 + 8;
    piVar3 = piVar3 + 1;
  } while (uVar2 < 0x58);
  if (*in_EDX != 0) {
    piVar3 = in_EDX + 0x23;
    uVar2 = 0;
    do {
      *piVar3 = *(int *)((int)&DAT_00260fa0 + uVar2);
      uVar2 = uVar2 + 4;
      piVar3 = piVar3 + 1;
    } while (uVar2 < 0xe4);
    in_EDX[3] = DAT_002611c0;
  }
  return;
}


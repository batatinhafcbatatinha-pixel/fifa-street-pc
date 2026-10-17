// callers: 0x19f010 0x19f110 0x1a0740 0x1a0790

void FUN_00254ce0(int param_1,int *param_2,int param_3)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  
  iVar3 = DAT_00261238;
  if (param_2 != (int *)0x0) {
    *param_2 = *param_2 + 0x80000;
  }
  if ((&DAT_00263700)[param_1 * 3] != 0) {
    *(undefined4 *)((&DAT_00263700)[param_1 * 3] + 8) = *(undefined4 *)(iVar3 + 0x2c);
    puVar1 = (uint *)(&DAT_00263700)[param_1 * 3];
    uVar2 = *puVar1;
    *puVar1 = uVar2 - 0x80000;
    if ((uVar2 - 0x80000 & 0x78ffff) == 0) {
      FUN_00257b40(puVar1);
    }
  }
  iVar3 = (&DAT_002636f8)[param_1 * 3];
  (&DAT_002636f8)[param_1 * 3] = param_3;
  (&DAT_00263700)[param_1 * 3] = param_2;
  if (param_3 == iVar3) {
    DAT_00260d98 = DAT_00260d98 | 0x40;
    return;
  }
  DAT_00260d98 = DAT_00260d98 | 0x70;
  return;
}


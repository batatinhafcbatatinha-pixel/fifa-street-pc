// callers: 0x194900 0x250ac0

void FUN_002509d0(uint param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  int *piVar2;
  uint uVar3;
  
  puVar1 = DAT_00261238;
  uVar3 = param_1;
  if (DAT_00261238[0x1e1] == 0) {
    uVar3 = param_1 + 1;
  }
  if ((uVar3 & 3) != 0) {
    piVar2 = (int *)*DAT_00261238;
    if ((int *)DAT_00261238[1] <= piVar2) {
      piVar2 = (int *)FUN_00252040();
    }
    *piVar2 = (uVar3 * 0x10 + param_2) * 4 + 0x41ad0;
    piVar2[1] = param_3;
    *puVar1 = piVar2 + 2;
  }
  (&DAT_00260da0)[param_1 * 0x20 + param_2] = param_3;
  return;
}


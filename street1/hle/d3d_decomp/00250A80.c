// callers: 0x250ac0

void FUN_00250a80(int param_1,int param_2)

{
  undefined4 *puVar1;
  int *piVar2;
  
  puVar1 = DAT_00261238;
  piVar2 = (int *)*DAT_00261238;
  if ((int *)DAT_00261238[1] <= piVar2) {
    piVar2 = (int *)FUN_00252040();
  }
  *piVar2 = param_1 * 4 + 0x40ae0;
  piVar2[1] = param_2;
  *puVar1 = piVar2 + 2;
  *(int *)(&DAT_00260e18 + param_1 * 0x80) = param_2;
  return;
}


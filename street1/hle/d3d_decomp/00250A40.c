// callers: 0x250ac0

void FUN_00250a40(int param_1,int param_2)

{
  uint *puVar1;
  int *piVar2;
  
  puVar1 = DAT_00261238;
  piVar2 = (int *)*DAT_00261238;
  if ((int *)DAT_00261238[1] <= piVar2) {
    piVar2 = (int *)FUN_00252040();
  }
  *piVar2 = param_1 * 0x40 + 0x41b24;
  piVar2[1] = param_2;
  *puVar1 = (uint)(piVar2 + 2);
  *(int *)(&DAT_00260e14 + param_1 * 0x80) = param_2;
  return;
}


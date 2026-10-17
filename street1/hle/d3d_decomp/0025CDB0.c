// callers: 0x19e4a0 0x19e810

void FUN_0025cdb0(int param_1,int param_2,int param_3,int param_4,int param_5)

{
  uint *puVar1;
  int *piVar2;
  int iVar3;
  
  puVar1 = DAT_00261238;
  piVar2 = (int *)*DAT_00261238;
  if ((int *)DAT_00261238[1] <= piVar2) {
    piVar2 = (int *)FUN_00252040();
  }
  if (param_1 == -1) {
    iVar3 = 0x1518;
  }
  else {
    iVar3 = (param_1 + 0x1a0) * 0x10;
  }
  *piVar2 = iVar3 + 0x100000;
  piVar2[1] = param_2;
  piVar2[2] = param_3;
  piVar2[3] = param_4;
  piVar2[4] = param_5;
  *puVar1 = (uint)(piVar2 + 5);
  return;
}


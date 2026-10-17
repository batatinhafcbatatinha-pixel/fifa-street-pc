// callers: 0x19e4a0 0x19e810

void FUN_0025ce10(int param_1,uint param_2)

{
  undefined4 *puVar1;
  int *piVar2;
  
  puVar1 = DAT_00261238;
  piVar2 = (int *)*DAT_00261238;
  if ((int *)DAT_00261238[1] <= piVar2) {
    piVar2 = (int *)FUN_00252040();
  }
  *piVar2 = param_1 * 4 + 0x41940;
  piVar2[1] = param_2 >> 0x10 & 0xff | (param_2 & 0xff) << 0x10 | param_2 & 0xff00ff00;
  *puVar1 = piVar2 + 2;
  return;
}


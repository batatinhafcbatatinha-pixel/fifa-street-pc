// callers: 0x19f200

uint FUN_00258020(int param_1,byte param_2)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  if ((param_2 & 0x10) == 0) {
    puVar2 = (undefined4 *)*DAT_00261238;
    if ((undefined4 *)DAT_00261238[1] <= puVar2) {
      puVar2 = (undefined4 *)FUN_00252040();
    }
    *puVar2 = 0x41710;
    puVar2[1] = 0;
    *puVar1 = puVar2 + 2;
  }
  if ((param_2 & 0xa0) == 0) {
    FUN_00252140(param_1);
  }
  return *(uint *)(param_1 + 4) | 0x80000000;
}


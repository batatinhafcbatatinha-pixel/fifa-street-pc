// callers: 0x19e4a0 0x19e810 0x25d6d0

void FUN_0025cd70(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = &LAB_0008187d_3 + param_1 * 8;
  puVar2[1] = param_2;
  puVar2[2] = param_3;
  *puVar1 = puVar2 + 3;
  return;
}


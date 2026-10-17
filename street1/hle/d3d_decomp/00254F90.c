// callers: 0x25d330 0x25d450

void FUN_00254f90(undefined4 param_1,undefined4 param_2)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x41e9c;
  puVar2[1] = 0;
  uVar3 = FUN_00254660(puVar2 + 2,param_1,param_2);
  *puVar1 = uVar3;
  return;
}


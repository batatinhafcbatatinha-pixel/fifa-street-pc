// callers: 0x195950 0x19a210

undefined4 *
FUN_0025cf50(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            uint param_5,undefined4 param_6,uint param_7)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  uVar1 = FUN_00258870(param_1,param_2,param_3,param_4,param_6,0,param_7 == 5,param_7 == 4,&param_7,
                       &param_6);
  if ((param_5 & 0x10000) != 0) {
    param_7 = param_7 & 0xfffffff7;
  }
  puVar2 = (undefined4 *)FUN_001609a7(0x14,0x64800000);
  if (puVar2 != (undefined4 *)0x0) {
    uVar3 = FUN_001609a7(uVar1,0xb7800000);
    if (uVar3 != 0) {
      puVar2[1] = uVar3 & 0xfffffff;
      *puVar2 = 0x1040001;
      puVar2[3] = param_7;
      puVar2[4] = param_6;
      puVar2[2] = 0;
      return puVar2;
    }
    FUN_00160a47(puVar2,0x24800000);
  }
  return (undefined4 *)0x0;
}


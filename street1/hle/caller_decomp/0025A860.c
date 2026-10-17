
undefined4 *
FUN_0025a860(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  uVar1 = FUN_00258870(param_1,param_2,1,1,param_4,0,0,0,&param_4,&param_2);
  puVar2 = (undefined4 *)FUN_001609a7(0x18,0x64800000);
  if (puVar2 != (undefined4 *)0x0) {
    uVar3 = FUN_001609a7(uVar1,0xb6800000);
    if (uVar3 != 0) {
      puVar2[1] = uVar3 & 0xfffffff;
      *puVar2 = 0x81050001;
      puVar2[3] = param_4;
      puVar2[4] = param_2;
      puVar2[5] = 0;
      puVar2[2] = 0;
      return puVar2;
    }
    FUN_00160a47(puVar2,0x24800000);
  }
  return (undefined4 *)0x0;
}


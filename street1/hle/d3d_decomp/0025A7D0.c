// callers: 0x25d010

undefined4 * FUN_0025a7d0(undefined4 param_1,undefined4 param_2,undefined4 param_3,uint param_4)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)FUN_001609a7(0x18,0x64800000);
  if (puVar1 == (undefined4 *)0x0) {
    return (undefined4 *)0x0;
  }
  puVar1[1] = param_4 & 0xfffffff;
  *puVar1 = 0x1050001;
  puVar1[3] = param_1;
  puVar1[4] = param_2;
  puVar1[5] = param_3;
  puVar1[2] = 0;
  FUN_00257c60(param_3);
  return puVar1;
}


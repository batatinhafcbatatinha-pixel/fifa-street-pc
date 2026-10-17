// callers: 0x19f340

undefined4 FUN_0025de50(undefined4 *param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  
  puVar1 = (undefined4 *)FUN_001609a7(0xfc,0x24800000);
  if (puVar1 == (undefined4 *)0x0) {
    return 0x8007000e;
  }
  puVar1[2] = puVar1 + 3;
  *puVar1 = 1;
  puVar1[1] = 1;
  puVar3 = puVar1 + 3;
  for (iVar2 = 0x3c; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = *param_1;
    param_1 = param_1 + 1;
    puVar3 = puVar3 + 1;
  }
  *param_2 = puVar1;
  return 0;
}


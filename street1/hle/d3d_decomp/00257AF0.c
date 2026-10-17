// callers: 0x257b40

void FUN_00257af0(int param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = &DAT_002639f0;
  puVar2 = DAT_002639f0;
  while (puVar2 != (undefined4 *)0x0) {
    if (puVar2[1] == param_1) {
      *puVar1 = *puVar2;
      if ((*(byte *)(puVar2 + 7) & 1) != 0) {
        (**(code **)(*(int *)puVar2[8] + 8))((int *)puVar2[8]);
      }
      FUN_00160a47(puVar2,0x24800000);
      puVar2 = (undefined4 *)*puVar1;
    }
    else {
      puVar1 = puVar2;
      puVar2 = (undefined4 *)*puVar2;
    }
  }
  return;
}


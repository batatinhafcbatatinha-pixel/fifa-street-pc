// callers: 0x18e540 0x19ae20

void FUN_00253c00(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x41710;
  puVar2[1] = 0;
  *puVar1 = puVar2 + 2;
  return;
}


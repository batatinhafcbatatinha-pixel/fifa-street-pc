// callers: 0x255340

void FUN_00255250(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0xc0180;
  puVar2[1] = 2;
  puVar2[2] = 3;
  puVar2[3] = 3;
  puVar2[4] = 0x180190;
  puVar2[5] = 4;
  puVar2[6] = 9;
  puVar2[7] = 10;
  puVar2[8] = 3;
  puVar2[9] = 3;
  puVar2[10] = 8;
  puVar2[0xb] = 0x401a8;
  puVar2[0xc] = 0xc;
  puVar2[0xd] = 0x41d6c;
  puVar2[0xe] = 0;
  puVar2 = puVar2 + 0xf;
  *puVar1 = puVar2;
  if ((undefined4 *)puVar1[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x409fc;
  puVar2[1] = 1;
  puVar2[2] = 0x100a50;
  puVar2[3] = 0;
  puVar2[4] = 0;
  puVar2[5] = 0;
  puVar2[6] = 0x3f800000;
  puVar2[7] = 0x416bc;
  puVar2[8] = 1;
  puVar2[9] = 0x41e78;
  puVar2[10] = 0x210000;
  puVar2[0xb] = 0x41d80;
  puVar2[0xc] = 1;
  puVar2[0xd] = 0x41e68;
  puVar2[0xe] = 0x7f800000;
  *puVar1 = puVar2 + 0xf;
  FUN_00254e70(0);
  return;
}


// callers: 0x255bf0

void FUN_00255340(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = DAT_00261238;
  puVar2 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar2) {
    puVar2 = (undefined4 *)FUN_00252040();
  }
  *puVar2 = 0x40000;
  puVar2[1] = 0xd;
  puVar2[2] = 0x42000;
  puVar2[3] = 0xe;
  puVar2[4] = 0x44000;
  puVar2[5] = 0x10;
  puVar2[6] = 0x46000;
  puVar2[7] = 0x11;
  puVar2[8] = &LAB_0004a000;
  puVar2[9] = 0x12;
  puVar2[10] = 0x42180;
  puVar2[0xb] = 7;
  puVar2[0xc] = 0x442fc;
  puVar2[0xd] = 3;
  puVar1[0x653] = 0;
  puVar1[0x654] = 0;
  puVar1[0x655] = 0;
  puVar1[0x656] = 0;
  puVar1[0x657] = 0;
  puVar1[0x658] = 0;
  puVar1[0x659] = 0;
  puVar1[0x65a] = 0;
  puVar1[0x64b] = 0;
  puVar1[0x64c] = 0;
  puVar1[0x64d] = 0;
  puVar1[0x64e] = 0;
  puVar1[0x64f] = 0;
  puVar1[0x650] = 0;
  puVar1[0x651] = 0;
  puVar1[0x652] = 0;
  puVar1[0x64b] = 0;
  puVar1[0x64c] = 3;
  puVar2[0xe] = 0x86184;
  puVar2[0xf] = 3;
  puVar2[0x10] = 0xb;
  puVar2[0x11] = 0x1c4184;
  puVar2[0x12] = 0x19;
  puVar2[0x13] = 0x19;
  puVar2[0x14] = 0x19;
  puVar2[0x15] = 0x19;
  puVar2[0x16] = 0x19;
  puVar2[0x17] = 0x19;
  puVar2[0x18] = 0x11;
  *puVar1 = puVar2 + 0x19;
  FUN_00255250();
  return;
}


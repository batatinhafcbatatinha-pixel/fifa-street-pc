// callers: 0x250f40 0x2512a0 0x251610 0x252af0 0x2541d0 0x2550e0 0x2569f0

void FUN_00254ff0(void)

{
  float fVar1;
  float fVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 *in_ECX;
  undefined *puVar5;
  int iVar6;
  
  if ((*(byte *)(in_ECX[0x1e5] + 4) & 2) == 0) {
    return;
  }
  if (DAT_00261114 == 0) {
    puVar5 = &DAT_0025f238;
  }
  else {
    if ((*(byte *)(in_ECX + 2) & 2) != 0) {
      iVar6 = 0x2c;
      puVar5 = &DAT_0025f188;
      goto LAB_00255034;
    }
    puVar5 = &DAT_0025f0c8;
  }
  iVar6 = 0x30;
LAB_00255034:
  puVar3 = (undefined4 *)FUN_00252050(iVar6 + 0x1e);
  *puVar3 = 0x41ea4;
  puVar3[1] = 0;
  puVar3[2] = 0x200b80;
  if (DAT_002611dc == 2) {
    fVar2 = (float)in_ECX[0x252] * (float)in_ECX[0x251];
  }
  else {
    fVar2 = 1.0;
  }
  puVar3[3] = in_ECX[599];
  puVar3[4] = in_ECX[600];
  uVar4 = in_ECX[0x252];
  puVar3[6] = fVar2;
  fVar2 = 0.0;
  puVar3[5] = uVar4;
  if (((char)((uint)in_ECX[2] >> 8) < '\0') && (DAT_00261200 != 0)) {
    fVar2 = 0.5;
  }
  puVar3[7] = (float)in_ECX[0x3be] - fVar2;
  fVar1 = (float)in_ECX[0x3bf];
  puVar3[9] = 0;
  puVar3[10] = 0;
  puVar3[8] = fVar1 - fVar2;
  puVar3[0xb] = 0x41e9c;
  puVar3[0xc] = 0;
  uVar4 = FUN_00254660(puVar3 + 0xd,puVar5,iVar6);
  *in_ECX = uVar4;
  return;
}


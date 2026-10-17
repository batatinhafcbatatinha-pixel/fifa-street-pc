// callers: 0x251d90 0x251f00 0x252e00 0x255bf0 0x25b070 0x25cb10 0x25cea0 0x25db50 0x25db90

uint FUN_00251cd0(byte param_1)

{
  uint uVar1;
  uint *puVar2;
  undefined4 *puVar3;
  uint uVar4;
  
  puVar2 = DAT_00261238;
  puVar3 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar3) {
    puVar3 = (undefined4 *)FUN_00252040();
  }
  uVar1 = puVar2[0xb];
  *puVar3 = FUN_0004a310;
  puVar3[1] = ((int)puVar3 * 8 | uVar1 & 0x1f) << 2 | puVar2[0x10] & 3;
  puVar3[2] = 0x41d70;
  puVar3[3] = uVar1;
  puVar3[4] = 0x41d90;
  puVar3[5] = 0;
  puVar3[6] = 0x41d90;
  puVar3[7] = 0;
  *puVar2 = (uint)(puVar3 + 8);
  uVar4 = uVar1 >> 1 & 0x3f;
  puVar2[uVar4 * 2 + 0x19] = uVar1;
  puVar2[uVar4 * 2 + 0x1a] = (uint)puVar3;
  if ((param_1 & 1) != 0) {
    uVar4 = puVar2[0xe] & puVar2[0xd] + 1;
    puVar2[0xd] = uVar4;
    *(uint *)(puVar2[0x12] + uVar4 * 8) = uVar1;
    *(undefined4 **)(puVar2[0x12] + 4 + uVar4 * 8) = puVar3;
  }
  puVar2[0xf] = 0;
  puVar2[0xb] = puVar2[0xb] + 2;
  if ((param_1 & 2) == 0) {
    FUN_00251bb0();
  }
  return uVar1;
}


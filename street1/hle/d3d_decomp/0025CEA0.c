// callers: 0x19e4a0 0x19e810 0x25d6d0

void FUN_0025cea0(void)

{
  uint uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = DAT_00261238;
  puVar3 = (undefined4 *)*DAT_00261238;
  if ((undefined4 *)DAT_00261238[1] <= puVar3) {
    puVar3 = (undefined4 *)FUN_00252040();
  }
  *puVar3 = 0x417fc;
  puVar3[1] = 0;
  *puVar2 = puVar3 + 2;
  uVar1 = puVar2[2];
  puVar2[2] = uVar1 & 0xffffe7ff;
  if ((uVar1 & 0x1000) != 0) {
    FUN_00251cd0(1);
  }
  return;
}


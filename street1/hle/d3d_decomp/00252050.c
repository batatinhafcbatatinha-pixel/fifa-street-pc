// callers: 0x251b80 0x254d70 0x254ff0 0x25b070 0x25ca70 0x25cb10

void FUN_00252050(int param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  if (DAT_00261238[1] + 0x200U <= (uint)(*DAT_00261238 + param_1 * 4)) {
    uVar1 = param_1 * 4 + 0x204;
    uVar2 = DAT_002639e8 >> 1;
    if (DAT_002639e8 >> 1 < uVar1) {
      uVar2 = uVar1;
    }
    uVar3 = DAT_002639e8;
    if (DAT_002639e8 < uVar1) {
      uVar3 = uVar1;
    }
    FUN_00251f00(uVar2,uVar3);
  }
  return;
}


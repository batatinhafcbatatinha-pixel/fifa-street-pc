// callers: 0x198750 0x1995d0 0x199c10 0x19a210 0x25e4a0 0x25e820

undefined4 FUN_00253230(int param_1)

{
  undefined4 uVar1;
  
  if (param_1 == -1) {
    uVar1 = *(undefined4 *)(DAT_00261238 + 0x1a18);
    FUN_00257c60(uVar1);
    return uVar1;
  }
  uVar1 = *(undefined4 *)(DAT_00261238 + 0x1a14 + (-(uint)(param_1 != 0) & 2) * 4);
  FUN_00257c60(uVar1);
  return uVar1;
}


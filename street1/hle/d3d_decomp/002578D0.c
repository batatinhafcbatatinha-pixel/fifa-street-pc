// callers: 0x257950 0x25ce60 0x25d330

void FUN_002578d0(void)

{
  undefined4 uVar1;
  uint uVar2;
  
  uVar1 = DAT_00261238;
  uVar2 = DAT_00260d98;
  if ((DAT_00260d98 & 0x100) != 0) {
    FUN_00256310(DAT_00261238);
  }
  if ((uVar2 & 0x800) != 0) {
    uVar2 = FUN_0025a9d0(uVar1,uVar2);
  }
  if ((uVar2 & 0x4000) != 0) {
    FUN_00256480(uVar1);
  }
  if ((uVar2 & 0xf) != 0) {
    FUN_00256800(uVar1,uVar2);
  }
  if ((uVar2 & 0x2000) != 0) {
    FUN_002569f0(uVar1);
  }
  if ((uVar2 & 0x400) != 0) {
    FUN_00256bd0(uVar1);
  }
  if ((uVar2 & 0xff1000) != 0) {
    FUN_00257380(uVar1,uVar2);
  }
  if ((uVar2 & 0x200) != 0) {
    FUN_002576d0(uVar1,uVar2);
  }
  DAT_00260d98 = uVar2 & 0xc0000070;
  return;
}


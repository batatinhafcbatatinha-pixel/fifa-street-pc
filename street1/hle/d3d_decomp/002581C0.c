// callers: 0x255650

uint FUN_002581c0(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = (((byte)(&DAT_0025ed88)[param_2] & 0x3c) * param_1 >> 3) + 0x3f & 0xffffffc0;
  uVar2 = 0;
  do {
    if (uVar1 <= (uint)(&DAT_0025ee90)[uVar2]) {
      return (&DAT_0025ee90)[uVar2];
    }
    uVar2 = uVar2 + 1;
  } while (uVar2 < 0x1a);
  return uVar1;
}


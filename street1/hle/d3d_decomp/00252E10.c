// callers: 0x252f00 0x252fb0 0x253280 0x2540a0 0x255650 0x255bf0 0x25e820

uint FUN_00252e10(int param_1)

{
  byte bVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x10) != 0) {
    return (*(byte *)(param_1 + 0x13) + 1) * 0x40;
  }
  bVar1 = *(byte *)(param_1 + 0xd);
  iVar2 = 1 << ((byte)(*(uint *)(param_1 + 0xc) >> 0x14) & 0xf);
  if (bVar1 != 0xc) {
    if ((0xd < bVar1) && (bVar1 < 0x10)) {
      return iVar2 * 4;
    }
    return ((byte)(&DAT_0025ed88)[*(byte *)(param_1 + 0xd)] & 0x3c) * iVar2 >> 3;
  }
  return iVar2 * 2;
}


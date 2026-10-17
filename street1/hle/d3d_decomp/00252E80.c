// callers: 0x253280

uint FUN_00252e80(int param_1)

{
  byte bVar1;
  char cVar2;
  uint uVar3;
  uint uVar4;
  char cVar5;
  
  bVar1 = (&DAT_0025ed88)[*(byte *)(param_1 + 0xd)];
  uVar4 = *(uint *)(param_1 + 0x10);
  cVar2 = FUN_00258170((uint)*(byte *)(param_1 + 0xd));
  uVar3 = -(uint)(cVar2 != '\0') & 2;
  if (uVar4 == 0) {
    uVar4 = *(uint *)(param_1 + 0xc) >> 0x14 & 0xf;
    cVar2 = (char)uVar4;
    if (uVar4 < uVar3) {
      cVar2 = (char)uVar3;
    }
    uVar4 = *(uint *)(param_1 + 0xc) >> 0x18 & 0xf;
    cVar5 = (char)uVar4;
    if (uVar4 < uVar3) {
      cVar5 = (char)uVar3;
    }
    return (1 << cVar5 + cVar2) * (bVar1 & 0x3c) >> 3;
  }
  return ((uVar4 >> 0xc & 0xfff) + 1) * ((uVar4 >> 0x18) + 1) * 0x40;
}


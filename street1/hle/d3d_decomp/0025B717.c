// callers: 0x25a356 0x25a544 0x25b756 0x25c5b8 0x25c75c

void FUN_0025b717(void)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int *in_ECX;
  
  iVar1 = *in_ECX;
  iVar2 = *(int *)(iVar1 + 0x400700);
  while (iVar2 != 0) {
    uVar3 = *(uint *)(iVar1 + 0x100);
    if ((uVar3 & 0x1000) != 0) {
      FUN_00259900();
    }
    if ((uVar3 & 0x1000000) != 0) {
      FUN_00259390();
    }
    iVar2 = *(int *)(iVar1 + 0x400700);
  }
  return;
}


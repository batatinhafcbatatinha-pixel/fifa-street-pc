// callers: 0x2599b0 0x259b40 0x25a041 0x25a606 0x25b717 0x25c5b8

undefined4 FUN_00259900(void)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int *in_ECX;
  uint uVar4;
  
  iVar1 = *in_ECX;
  *(undefined4 *)(iVar1 + 0x400720) = 0;
  uVar3 = *(uint *)(iVar1 + 0x400100);
  uVar2 = *(uint *)(iVar1 + 0x400108);
  uVar4 = *(uint *)(iVar1 + 0x400704) & 0x1ffc;
  if ((uVar3 & 0x1000) != 0) {
    *(undefined4 *)(iVar1 + 0x400100) = 0x1000;
    do {
    } while (*(int *)(*in_ECX + 0x400700) != 0);
    FUN_0025c5b8(0);
    uVar3 = *(uint *)(iVar1 + 0x400100);
    if (uVar3 == 0) goto LAB_00259996;
  }
  *(uint *)(iVar1 + 0x400100) = uVar3;
  if (((uVar2 != 0) && ((uVar3 & 0x100001) != 0)) && ((uVar2 & 0x40) == 0)) {
    if (uVar4 == 0x100) {
      FUN_00259690(*(undefined4 *)(iVar1 + 0x400708));
    }
    else {
      FUN_00259640(0,uVar2,uVar4,*(undefined4 *)(iVar1 + 0x400708));
    }
  }
LAB_00259996:
  *(undefined4 *)(iVar1 + 0x400720) = 1;
  return *(undefined4 *)(iVar1 + 0x400100);
}


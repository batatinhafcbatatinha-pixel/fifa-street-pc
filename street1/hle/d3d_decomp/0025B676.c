// callers: 0x259d6d

void FUN_0025b676(void)

{
  uint uVar1;
  undefined4 *puVar2;
  int *in_ECX;
  int iVar3;
  
  iVar3 = *in_ECX;
  uVar1 = in_ECX[0x50];
  *(uint *)(iVar3 + 0x400780) = uVar1 & 0xffff;
  puVar2 = (undefined4 *)((uVar1 + 0x70000) * 0x10 + iVar3);
  iVar3 = 2;
  do {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}


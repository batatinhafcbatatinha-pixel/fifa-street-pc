// callers: 0x25a1f4

undefined4 FUN_00259df0(void)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  int *in_ECX;
  
  iVar2 = *in_ECX;
  puVar1 = (uint *)(iVar2 + 0x184c);
  uVar3 = *puVar1;
  *puVar1 = uVar3 & 0xfffffcff;
  *puVar1 = uVar3;
  *(undefined4 *)(iVar2 + 0x200) = 0xffffffff;
  *(int *)(iVar2 + 0x140) = in_ECX[0x2d];
  FUN_0025b98c();
  *(undefined4 *)(iVar2 + 0x9400) = 0;
  *(undefined4 *)(iVar2 + 0x9410) = 0;
  *(undefined4 *)(iVar2 + 0x400720) = 0;
  FUN_0025c75c();
  *(undefined4 *)(iVar2 + 0x400100) = 0xffffffff;
  *(undefined4 *)(iVar2 + 0x400140) = 0xffffffff;
  FUN_0025c6db();
  *(undefined4 *)(iVar2 + 0x2100) = 0xffffffff;
  *(int *)(iVar2 + 0x2140) = in_ECX[0x47];
  in_ECX[0x210] = -1;
  in_ECX[0x211] = -1;
  in_ECX[0x208] = 0;
  in_ECX[0x209] = 0;
  return 1;
}


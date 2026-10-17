// callers: 0x257380

uint FUN_00256270(void)

{
  uint uVar1;
  uint uVar2;
  
  uVar2 = 0;
  if (DAT_00261144 != 0) {
    uVar2 = ((((((DAT_00261148 << 2 | DAT_0026114c) << 2 | DAT_00261150) << 2 | DAT_00261154) << 2 |
              DAT_00261158) << 2 | DAT_0026115c) << 2 | DAT_00261160) << 2 | DAT_00261164;
    uVar1 = *(uint *)(*(int *)(DAT_00261238 + 0x794) + 4);
    if ((uVar1 & 0x400) == 0) {
      uVar2 = uVar2 & 0xffffffaa;
    }
    if ((uVar1 & 0x800) == 0) {
      uVar2 = uVar2 & 0xffffff55;
    }
    if ((uVar1 & 0x1000) == 0) {
      uVar2 = uVar2 & 0xffffaaff;
    }
    if ((uVar1 & 0x2000) == 0) {
      uVar2 = uVar2 & 0xffff55ff;
    }
  }
  return uVar2;
}


// callers: 0x198750 0x199930

undefined4 FUN_002540a0(void)

{
  undefined1 uVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  int iVar9;
  int iVar10;
  undefined4 uVar11;
  
  iVar10 = DAT_00261238;
  iVar4 = (*(code *)0x80000001)();
  if (iVar4 != 0) {
    (*(code *)0x800000ab)(iVar4);
    (*(code *)0x80000004)(0);
  }
  if (*(int *)(iVar10 + 0x938) == 0) {
    return 0x80004005;
  }
  do {
    iVar4 = FUN_00259160();
  } while (iVar4 != 0);
  iVar4 = *(int *)(iVar10 + 0x1a18);
  uVar1 = *(undefined1 *)(iVar4 + 0xd);
  uVar2 = *(uint *)(iVar4 + 0x10);
  if (uVar2 == 0) {
    iVar9 = 1 << ((byte)(*(uint *)(iVar4 + 0xc) >> 0x14) & 0xf);
    iVar5 = 1 << (*(byte *)(iVar4 + 0xf) & 0xf);
  }
  else {
    iVar9 = (uVar2 & 0xfff) + 1;
    iVar5 = (uVar2 >> 0xc & 0xfff) + 1;
  }
  iVar9 = FUN_00254020(iVar9,iVar5,uVar1);
  if (iVar9 != 0) {
    FUN_00253280(iVar4,0,0,iVar9,0);
    FUN_002520a0();
    uVar8 = *(undefined4 *)(iVar10 + 0x1c28);
    uVar3 = *(undefined4 *)(iVar10 + 0x1c30);
    iVar10 = 0;
    do {
      uVar11 = *(undefined4 *)(iVar9 + 4);
      uVar6 = FUN_00252e10(iVar9);
      uVar7 = FUN_002580b0(uVar1);
      iVar10 = (*(code *)0x80000003)(uVar8,iVar10,uVar3,uVar7,uVar6,uVar11);
    } while (iVar10 != 0);
    DAT_002636ec = iVar10;
    DAT_002636f0 = iVar10;
    FUN_0025a1ad();
    uVar8 = (*(code *)0x800000b4)(iVar9);
    (*(code *)0x800000b2)(iVar9,uVar8,1);
    (*(code *)0x80000004)(iVar9);
    return 0;
  }
  return 0x80004005;
}


// callers: 0x25db50

void FUN_0025d210(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint unaff_EBX;
  uint uVar5;
  int iVar6;
  int unaff_EDI;
  undefined1 *puVar7;
  undefined1 auStack_4 [4];
  
  uVar4 = *(undefined4 *)(unaff_EDI + 0x1c34);
  uVar1 = *(undefined4 *)(unaff_EDI + 0x1c28);
  uVar5 = *(uint *)(unaff_EDI + 0x1c2c);
  uVar2 = *(undefined4 *)(unaff_EDI + 0x1c30);
  FUN_002520a0();
  if ((*(uint *)(unaff_EDI + 0x1ddc) & 0x1000000) != 0) {
    uVar5 = uVar5 >> 1;
  }
  uVar3 = *(undefined4 *)(*(int *)(unaff_EDI + 0x1a18) + 4);
  iVar6 = 0;
  do {
    FUN_00252ce0();
    iVar6 = (*(code *)0x80000003)(uVar1,iVar6,uVar2,uVar4,uVar5,uVar3);
  } while (iVar6 != 0);
  if ((*(uint *)(unaff_EDI + 0x1ddc) & 0x1000000) != 0) {
    (*(code *)0x80000002)(*(undefined4 *)(unaff_EDI + 0x1c28),0xb,0,0);
  }
  puVar7 = auStack_4;
  (*(code *)0x80000002)(*(undefined4 *)(unaff_EDI + 0x1c28),0xf,0,puVar7);
  if (((*(uint *)(unaff_EDI + 0x1de8) ^ unaff_EBX) & 1) == 0) {
    *(uint *)(unaff_EDI + 0x1de8) = *(uint *)(unaff_EDI + 0x1de8) + 1;
  }
  DAT_002636f0 = 0;
  DAT_002636ec = 0;
  FUN_0025a1ad();
  iVar6 = (*(code *)0x80000001)();
  if (iVar6 != 0) {
    uVar4 = (*(code *)0x800000b4)(iVar6);
    (*(code *)0x800000b2)(iVar6,uVar4,0);
    (*(code *)0x800000ab)(iVar6);
    (*(code *)0x80000004)(0);
  }
  (*(code *)0x80000002)(puVar7,9,0,0);
  *(undefined4 *)(unaff_EDI + 0x1de0) = 0;
  return;
}


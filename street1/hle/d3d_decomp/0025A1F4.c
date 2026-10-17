// callers: 0x255bf0

bool FUN_0025a1f4(void)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  uint uVar4;
  int in_ECX;
  undefined1 *puVar5;
  undefined1 local_14 [4];
  int local_10;
  undefined1 local_c [3];
  byte local_9;
  int local_8;
  
  (*(code *)0x8000006b)(in_ECX + 0x84,FUN_00259b40);
  iVar3 = in_ECX + 0x1ac;
  *(int *)(in_ECX + 0x1b0) = iVar3;
  *(int *)iVar3 = iVar3;
  iVar3 = in_ECX + 0x19c;
  *(undefined1 *)(in_ECX + 0x1a4) = 0;
  *(undefined1 *)(in_ECX + 0x1a6) = 4;
  *(undefined4 *)(in_ECX + 0x1a8) = 1;
  *(undefined1 *)(in_ECX + 0x194) = 0;
  *(undefined1 *)(in_ECX + 0x196) = 4;
  *(undefined4 *)(in_ECX + 0x198) = 1;
  *(int *)(in_ECX + 0x1a0) = iVar3;
  *(int *)iVar3 = iVar3;
  iVar3 = FUN_00259d01();
  if ((iVar3 != 0) && (iVar3 = FUN_00259d2c(), iVar3 != 0)) {
    (*(code *)0x8000002c)(3,local_14);
    (*(code *)0x8000006d)(in_ECX + 0x14,FUN_00259190);
    cVar1 = (*(code *)0x80000062)(in_ECX + 0x14);
    if (cVar1 != '\0') {
      *(undefined4 *)(in_ECX + 0x168) = 0;
      *(undefined4 *)(in_ECX + 0x164) = &LAB_002595d0;
      (*(code *)0x8000002f)((undefined4 *)(in_ECX + 0x164),1);
      iVar3 = FUN_00259d6d();
      if (iVar3 != 0) {
        local_8 = in_ECX + 0x2dc;
        local_10 = 2;
        do {
          uVar4 = 0;
          do {
            puVar5 = (undefined1 *)(local_8 + uVar4);
            uVar2 = (undefined1)uVar4;
            puVar5[-0x100] = uVar2;
            *puVar5 = uVar2;
            puVar5[0x100] = uVar2;
            uVar4 = uVar4 + 1;
          } while (uVar4 < 0x100);
          local_8 = local_8 + 0x300;
          local_10 = local_10 + -1;
        } while (local_10 != 0);
        out(0x80c0,1);
        (*(code *)0x8000002e)(1,0,0x4c,local_c,4,0);
        local_9 = local_9 | 0x1f;
        (*(code *)0x8000002e)(1,0,0x4c,local_c,4,1);
        *(undefined4 *)(in_ECX + 0xa0) = 1;
        iVar3 = FUN_00259df0();
        return iVar3 != 0;
      }
    }
  }
  return false;
}


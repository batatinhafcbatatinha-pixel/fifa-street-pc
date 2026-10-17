// callers: 0x2594a0 0x2599b0 0x259b40 0x25a041 0x25a606 0x25b717

undefined4 FUN_00259390(void)

{
  undefined1 uVar1;
  int iVar2;
  byte bVar3;
  int iVar4;
  uint uVar5;
  int *in_ECX;
  
  iVar2 = *in_ECX;
  iVar4 = FUN_00259180();
  if (in_ECX[0x76] != 0) {
    in_ECX[0x75] = iVar4 - in_ECX[0x76];
  }
  in_ECX[0x76] = iVar4;
  in_ECX[0x70] = in_ECX[0x70] + 1;
  uVar1 = *(undefined1 *)(iVar2 + 0x6013d4);
  iVar4 = FUN_002592b0();
  if ((iVar4 == 0) && (in_ECX[0x70] == in_ECX[0x71])) {
    in_ECX[0x71] = in_ECX[0x71] + 1;
  }
  if ((in_ECX[0x6d] & 0x1000000U) == 0) {
    bVar3 = in(0x80c0);
    uVar5 = ~(uint)(bVar3 >> 5) & 1;
    in_ECX[0x74] = uVar5;
    if ((in_ECX[2] & 0xc0000000U) == 0x40000000) {
      in_ECX[0x74] = (uint)(uVar5 == 0);
    }
  }
  do {
    *(undefined4 *)(iVar2 + 0x600100) = 1;
  } while ((*(uint *)(iVar2 + 0x100) & 0x1000000) != 0);
  (*(code *)0x80000091)(in_ECX + 0x65,1,0);
  if ((code *)in_ECX[100] != (code *)0x0) {
    (*(code *)in_ECX[100])(&stack0xffffffe8);
  }
  *(undefined1 *)(iVar2 + 0x6013d4) = uVar1;
  return 0;
}


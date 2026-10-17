// callers: 0x259b40

undefined4 FUN_002594a0(void)

{
  code *pcVar1;
  int *in_ECX;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined8 uVar5;
  
  *(undefined4 *)(*in_ECX + 0x9100) = 1;
  iVar2 = in_ECX[0x210];
  if ((iVar2 == -1) || (in_ECX[iVar2 * 2 + 0x20c] == 0 && in_ECX[iVar2 * 2 + 0x20d] == 0)) {
    FUN_0025a0c1(0);
    return 0;
  }
  iVar4 = 0;
  while( true ) {
    uVar3 = iVar2 + 1U & 0x80000001;
    if ((int)uVar3 < 0) {
      uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
    }
    in_ECX[0x210] = uVar3;
    in_ECX[iVar2 * 2 + 0x20c] = 0;
    in_ECX[iVar2 * 2 + 0x20d] = 0;
    if (iVar2 == in_ECX[0x211]) {
      pcVar1 = (code *)in_ECX[0x212];
      in_ECX[0x211] = -1;
      if (pcVar1 != (code *)0x0) {
        in_ECX[0x212] = 0;
        (*pcVar1)(in_ECX[0x213]);
      }
    }
    else {
      uVar3 = in_ECX[0x208];
      if ((in_ECX[0x209] != 0) || (uVar3 != 0)) {
        iVar2 = in_ECX[0x20b] + in_ECX[0x209] + (uint)CARRY4(in_ECX[0x20a],uVar3);
        in_ECX[0x20a] = in_ECX[0x20a] + uVar3;
        in_ECX[0x20b] = iVar2;
        FUN_0025a0df(in_ECX[0x20a],iVar2,1);
        FUN_00259390();
      }
    }
    iVar2 = in_ECX[0x210];
    if (in_ECX[iVar2 * 2 + 0x20c] == 0 && in_ECX[iVar2 * 2 + 0x20d] == 0) {
      return 0;
    }
    uVar5 = FUN_0025a0ac();
    uVar3 = (uint)((ulonglong)uVar5 >> 0x20);
    if (uVar3 < (uint)in_ECX[iVar2 * 2 + 0x20d]) {
      return 0;
    }
    if ((uVar3 <= (uint)in_ECX[iVar2 * 2 + 0x20d]) &&
       ((uint)uVar5 < (uint)in_ECX[iVar2 * 2 + 0x20c])) break;
    iVar4 = iVar4 + 1;
    if (1 < iVar4) {
      return 0;
    }
  }
  return 0;
}


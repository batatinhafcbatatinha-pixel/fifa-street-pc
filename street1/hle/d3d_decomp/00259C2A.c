// callers: 0x259d6d

void FUN_00259c2a(int param_1)

{
  int iVar1;
  undefined1 *puVar2;
  int iVar3;
  undefined1 uVar4;
  int in_ECX;
  uint uVar5;
  
  iVar3 = param_1 * 0x300 + 0x1dc + in_ECX;
  iVar1 = iVar3 + 0x100;
  uVar5 = 0;
  do {
    puVar2 = (undefined1 *)(iVar1 + uVar5);
    uVar4 = (undefined1)uVar5;
    puVar2[iVar3 - iVar1] = uVar4;
    *puVar2 = uVar4;
    puVar2[(iVar3 - iVar1) + 0x200] = uVar4;
    uVar5 = uVar5 + 1;
  } while (uVar5 < 0x100);
  return;
}


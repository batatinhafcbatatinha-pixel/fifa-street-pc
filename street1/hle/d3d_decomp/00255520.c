// callers: 0x252f00 0x2560c0

void FUN_00255520(void)

{
  int iVar1;
  int in_ECX;
  uint uVar2;
  int *piVar3;
  
  iVar1 = (*(code *)0x80000001)();
  if (iVar1 == 0) {
    (*(code *)0x80000002)(*(undefined4 *)(in_ECX + 0x1c28),9,1,0);
  }
  iVar1 = *(int *)(in_ECX + 0x1a04);
  if (iVar1 != 0) {
    if (*(int *)(iVar1 + 0x14) != 0) {
      iVar1 = *(int *)(iVar1 + 0x14);
    }
    *(undefined4 *)(iVar1 + 8) = *(undefined4 *)(in_ECX + 0x2c);
    FUN_00257e50(*(undefined4 *)(in_ECX + 0x1a04));
    *(undefined4 *)(in_ECX + 0x1a04) = 0;
  }
  iVar1 = *(int *)(in_ECX + 0x1a08);
  if (iVar1 != 0) {
    if (*(int *)(iVar1 + 0x14) != 0) {
      iVar1 = *(int *)(iVar1 + 0x14);
    }
    *(undefined4 *)(iVar1 + 8) = *(undefined4 *)(in_ECX + 0x2c);
    FUN_00257e50(*(undefined4 *)(in_ECX + 0x1a08));
    *(undefined4 *)(in_ECX + 0x1a08) = 0;
  }
  uVar2 = 0;
  if (*(int *)(in_ECX + 0x1a10) != 0) {
    piVar3 = (int *)(in_ECX + 0x1a14);
    do {
      if (*piVar3 != 0) {
        FUN_00257ca0(*piVar3);
        *piVar3 = 0;
      }
      uVar2 = uVar2 + 1;
      piVar3 = piVar3 + 1;
    } while (uVar2 < *(uint *)(in_ECX + 0x1a10));
  }
  if (*(int *)(in_ECX + 0x1a20) != 0) {
    FUN_00257ca0(*(int *)(in_ECX + 0x1a20));
    *(undefined4 *)(in_ECX + 0x1a20) = 0;
  }
  if (*(int *)(in_ECX + 0x1a8c) != 0) {
    FUN_00253c30(1,0);
    FUN_00160a47(*(undefined4 *)(in_ECX + 0x1a8c),0xa1800000);
    *(undefined4 *)(in_ECX + 0x1a8c) = 0;
  }
  if (*(int *)(in_ECX + 0x1a84) != 0) {
    FUN_00253c30(0,0);
    FUN_00160a47(*(undefined4 *)(in_ECX + 0x1a84),0xa1800000);
    *(undefined4 *)(in_ECX + 0x1a84) = 0;
  }
  if (*(int *)(in_ECX + 0x1a88) != 0) {
    FUN_00160a47(*(int *)(in_ECX + 0x1a88),0xa1800000);
    *(undefined4 *)(in_ECX + 0x1a88) = 0;
  }
  return;
}


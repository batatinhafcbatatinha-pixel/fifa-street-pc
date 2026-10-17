// callers: 0x25d850 0x25dd90

void FUN_0025d5d0(void)

{
  int iVar1;
  int in_EAX;
  int iVar2;
  int *piVar3;
  
  *(undefined4 *)(in_EAX + 0x1a90) = *(undefined4 *)(in_EAX + 0x1a04);
  FUN_00257c60(*(undefined4 *)(in_EAX + 0x1a04));
  iVar2 = *(int *)(in_EAX + 0x1a08);
  *(int *)(in_EAX + 0x1a94) = iVar2;
  if (iVar2 != 0) {
    FUN_00257c60(iVar2);
  }
  piVar3 = (int *)(in_EAX + 0x1a98);
  iVar2 = 4;
  do {
    iVar1 = piVar3[-0x2c4];
    *piVar3 = iVar1;
    if (iVar1 != 0) {
      FUN_00257c60(iVar1);
    }
    piVar3 = piVar3 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  *(undefined4 *)(in_EAX + 0x1aa8) = *(undefined4 *)(in_EAX + 0xee0);
  *(undefined4 *)(in_EAX + 0x1aac) = *(undefined4 *)(in_EAX + 0xee4);
  *(undefined4 *)(in_EAX + 0x1ab0) = *(undefined4 *)(in_EAX + 0xee8);
  *(undefined4 *)(in_EAX + 0x1ab4) = *(undefined4 *)(in_EAX + 0xeec);
  *(undefined4 *)(in_EAX + 0x1ab8) = *(undefined4 *)(in_EAX + 0xef0);
  *(undefined4 *)(in_EAX + 0x1abc) = *(undefined4 *)(in_EAX + 0xef4);
  return;
}


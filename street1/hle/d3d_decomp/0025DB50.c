// callers: 0x25db90

void FUN_0025db50(void)

{
  int iVar1;
  int in_EAX;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = *(int *)(*(int *)(in_EAX + 0x1a18) + 0x14);
  iVar2 = *(int *)(in_EAX + 0x1a18);
  if (iVar1 != 0) {
    iVar2 = iVar1;
  }
  *(undefined4 *)(iVar2 + 8) = *(undefined4 *)(in_EAX + 0x2c);
  uVar3 = FUN_00251cd0(0);
  *(undefined4 *)(in_EAX + 0x1974 + (*(uint *)(in_EAX + 0x2478) & 1) * 4) = uVar3;
  if (*(int *)(in_EAX + 0x1de0) != 0) {
    FUN_0025d210();
  }
  return;
}


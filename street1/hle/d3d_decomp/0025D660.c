// callers: 0x25d850 0x25dd90

void FUN_0025d660(void)

{
  int in_EAX;
  int *piVar1;
  uint uVar2;
  
  FUN_00252fb0(*(undefined4 *)(in_EAX + 0x1a90),*(undefined4 *)(in_EAX + 0x1a94));
  FUN_002535d0(in_EAX + 0x1aa8);
  FUN_00257ca0(*(undefined4 *)(in_EAX + 0x1a90));
  if (*(int *)(in_EAX + 0x1a94) != 0) {
    FUN_00257ca0(*(int *)(in_EAX + 0x1a94));
  }
  uVar2 = 0;
  piVar1 = (int *)(in_EAX + 0x1a98);
  do {
    FUN_00253800(uVar2,*piVar1);
    if (*piVar1 != 0) {
      FUN_00257ca0(*piVar1);
    }
    uVar2 = uVar2 + 1;
    piVar1 = piVar1 + 1;
  } while (uVar2 < 4);
  return;
}


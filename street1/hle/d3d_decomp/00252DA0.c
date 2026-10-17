// callers: 0x255bf0

void FUN_00252da0(int param_1)

{
  if ((DAT_002636ec == 0) || (DAT_002636e4 != (param_1 != 0))) {
    (*(code *)0x80000002)(*(undefined4 *)(DAT_00261238 + 0x1c28),0xe,param_1,0);
    DAT_002636e4 = (uint)(param_1 != 0);
    DAT_002636ec = 1;
  }
  return;
}


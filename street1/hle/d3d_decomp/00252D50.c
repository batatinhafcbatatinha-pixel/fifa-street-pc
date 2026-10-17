// callers: 0x255bf0

void FUN_00252d50(int param_1)

{
  if ((DAT_002636f0 == 0) || (DAT_002636e8 != param_1)) {
    (*(code *)0x80000002)(*(undefined4 *)(DAT_00261238 + 0x1c28),0xb,param_1,0);
    DAT_002636f0 = 1;
    DAT_002636e8 = param_1;
  }
  return;
}


// callers: 0x18e540

void FUN_0025cef0(undefined4 param_1,uint param_2)

{
  int iVar1;
  
  if (param_2 < 0x1001) {
    FUN_001609a7();
    return;
  }
  iVar1 = 0;
  if (param_2 != 0) {
    for (; (param_2 >> iVar1 & 1) == 0; iVar1 = iVar1 + 1) {
    }
  }
  FUN_001609a7();
  return;
}


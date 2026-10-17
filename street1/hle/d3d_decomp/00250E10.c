// callers: 0x2512a0 0x251440 0x252fb0

undefined4 * FUN_00250e10(void)

{
  char cVar1;
  int in_ECX;
  undefined4 *in_EDX;
  uint uVar2;
  
  uVar2 = 0x100001;
  if (DAT_00261220 != 0) {
    uVar2 = 0x10100001;
  }
  if (DAT_002611dc == 2) {
    uVar2 = uVar2 | 0x10000;
  }
  if ((*(int *)(in_ECX + 0x1a08) != 0) &&
     ((((cVar1 = *(char *)(*(int *)(in_ECX + 0x1a08) + 0xd), cVar1 == '-' || (cVar1 == '+')) ||
       (cVar1 == '1')) || (cVar1 == '/')))) {
    uVar2 = uVar2 | 0x1000;
  }
  *in_EDX = 0x40290;
  in_EDX[1] = uVar2;
  return in_EDX + 2;
}


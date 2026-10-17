// callers: 0x259d6d

void FUN_0025b640(void)

{
  undefined4 *puVar1;
  int *in_ECX;
  int iVar2;
  
  puVar1 = (undefined4 *)(*in_ECX + 0x8918);
  iVar2 = 2;
  do {
    puVar1[-2] = 0x1000;
    *puVar1 = 0x1000;
    puVar1[8] = 0x100000;
    puVar1[10] = 0x100000;
    puVar1[6] = 0;
    puVar1[4] = 0xffffffff;
    puVar1 = puVar1 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}


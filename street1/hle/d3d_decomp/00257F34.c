// callers: 0x2592b0

void FUN_00257f34(int param_1)

{
  undefined1 *puVar1;
  int iVar2;
  undefined1 *puVar3;
  int *in_ECX;
  
  iVar2 = *in_ECX;
  *(undefined1 *)(iVar2 + 0x6813c8) = 0;
  puVar3 = (undefined1 *)(param_1 + 0x100);
  do {
    *(undefined1 *)(iVar2 + 0x6813c9) = puVar3[-0x100];
    *(undefined1 *)(iVar2 + 0x6813c9) = *puVar3;
    puVar1 = puVar3 + 0x100;
    puVar3 = puVar3 + 1;
    *(undefined1 *)(iVar2 + 0x6813c9) = *puVar1;
  } while (puVar3 + (-0x100 - param_1) < (undefined1 *)0x100);
  return;
}


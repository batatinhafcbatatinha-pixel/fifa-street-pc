// callers: 0x259d6d

void FUN_0025b571(void)

{
  uint *puVar1;
  char cVar2;
  int iVar3;
  int *in_ECX;
  
  iVar3 = *in_ECX;
  *(undefined1 *)(iVar3 + 0x6013d4) = 0x1f;
  cVar2 = *(char *)(iVar3 + 0x6013d5);
  *(undefined1 *)(iVar3 + 0x6013d4) = 0x1f;
  *(undefined1 *)(iVar3 + 0x6013d5) = 0x57;
  *(undefined1 *)(iVar3 + 0x6013d4) = 0x52;
  *(undefined1 *)(iVar3 + 0x6013d4) = 0x52;
  *(char *)(iVar3 + 0x6013d5) = *(char *)(iVar3 + 0x6013d5) + '\x04';
  *(uint *)(iVar3 + 0x8088) = *(uint *)(iVar3 + 0x8088) & 0xfffff43f | 0x400;
  *(uint *)(iVar3 + 0x8088) = *(uint *)(iVar3 + 0x8088) & 0xf43fffff | 0x4000000;
  puVar1 = (uint *)(iVar3 + 0x808c);
  *puVar1 = *puVar1 & 0xfffff40f | 0x400;
  *puVar1 = *puVar1 & 0xf40fffff | 0x4000000;
  *(undefined1 *)(iVar3 + 0x6013d4) = 0x20;
  *(undefined1 *)(iVar3 + 0x6013d5) = 0x29;
  *(undefined1 *)(iVar3 + 0x6013d4) = 0x1b;
  *(undefined1 *)(iVar3 + 0x6013d5) = 5;
  *(uint *)(iVar3 + 0x600804) = *(uint *)(iVar3 + 0x600804) & 0xfffffffa | 2;
  if (cVar2 == '\0') {
    *(undefined1 *)(iVar3 + 0x6013d4) = 0x1f;
    *(undefined1 *)(iVar3 + 0x6013d5) = 0x99;
  }
  return;
}


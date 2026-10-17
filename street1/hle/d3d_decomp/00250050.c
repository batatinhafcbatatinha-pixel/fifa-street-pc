// callers: 0x1934b0 0x198750 0x198fc0 0x199120 0x199190 0x199260 0x1992d0 0x199340 0x1993b0 0x199600 0x250080

void FUN_00250050(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 in_ECX;
  undefined4 in_EDX;
  
  while( true ) {
    puVar1 = DAT_00261240;
    if (DAT_00261240 + 2 < DAT_00261244) break;
    FUN_00252040();
  }
  puVar2 = DAT_00261240 + 2;
  *DAT_00261240 = in_ECX;
  DAT_00261240 = puVar2;
  puVar1[1] = in_EDX;
  return;
}


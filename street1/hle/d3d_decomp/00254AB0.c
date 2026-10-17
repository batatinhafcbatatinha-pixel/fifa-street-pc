// callers: 0x1a0700

void FUN_00254ab0(void)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int in_ECX;
  int iVar4;
  undefined4 *in_EDX;
  
  while( true ) {
    puVar2 = DAT_00261240;
    if (DAT_00261240 + 7 < DAT_00261244) break;
    FUN_00252040();
  }
  puVar3 = DAT_00261240 + 7;
  *DAT_00261240 = 0x41ea4;
  DAT_00261240 = puVar3;
  puVar2[1] = in_ECX;
  puVar2[2] = 0x100b80;
  iVar4 = in_ECX * 0x10;
  uVar1 = *in_EDX;
  puVar2[3] = uVar1;
  *(undefined4 *)(&DAT_00260180 + iVar4) = uVar1;
  uVar1 = in_EDX[1];
  puVar2[4] = uVar1;
  *(undefined4 *)(&DAT_00260184 + iVar4) = uVar1;
  uVar1 = in_EDX[2];
  puVar2[5] = uVar1;
  *(undefined4 *)(&DAT_00260188 + iVar4) = uVar1;
  uVar1 = in_EDX[3];
  puVar2[6] = uVar1;
  *(undefined4 *)(&DAT_0026018c + iVar4) = uVar1;
  return;
}


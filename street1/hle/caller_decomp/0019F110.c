
void FUN_0019f110(void)

{
  undefined4 *puVar1;
  undefined4 *in_ECX;
  int iVar2;
  
  iVar2 = 0;
  if (*(char *)(in_ECX + 10) != '\0') {
    puVar1 = in_ECX + 4;
    do {
      if ((undefined4 *)(&DAT_002f49a0)[iVar2] == puVar1) {
        (&DAT_002f49a0)[iVar2] = 0;
        FUN_00254ce0(iVar2,0,0);
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < 0x10);
    thunk_FUN_002520c0(puVar1);
    *puVar1 = 0;
    in_ECX[5] = 0;
    in_ECX[6] = 0;
    *in_ECX = 0;
    return;
  }
  do {
    if ((&DAT_002f49a0)[iVar2] == in_ECX[1]) {
      (&DAT_002f49a0)[iVar2] = 0;
      FUN_00254ce0(iVar2,0,0);
    }
    if ((&DAT_002f49a0)[iVar2] == in_ECX[2]) {
      (&DAT_002f49a0)[iVar2] = 0;
      FUN_00254ce0(iVar2,0,0);
    }
    if ((&DAT_002f49a0)[iVar2] == in_ECX[3]) {
      (&DAT_002f49a0)[iVar2] = 0;
      FUN_00254ce0(iVar2,0,0);
    }
    iVar2 = iVar2 + 1;
  } while (iVar2 < 0x10);
  thunk_FUN_002520c0(in_ECX[1]);
  thunk_FUN_002520c0(in_ECX[2]);
  thunk_FUN_002520c0(in_ECX[3]);
  FUN_00257ca0(in_ECX[1]);
  FUN_00257ca0(in_ECX[2]);
  FUN_00257ca0(in_ECX[3]);
  in_ECX[1] = 0;
  in_ECX[2] = 0;
  in_ECX[3] = 0;
  *in_ECX = 0;
  return;
}


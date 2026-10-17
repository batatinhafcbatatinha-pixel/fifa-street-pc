// callers: 0x25d080

void FUN_002560c0(void)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  int *in_ECX;
  uint uVar4;
  int *piVar5;
  undefined4 *puVar6;
  
  if (*in_ECX != 0) {
    FUN_002520a0();
    *(uint *)*in_ECX = (in_ECX[9] & 0xfffffffU) + 1;
    *in_ECX = in_ECX[9];
    FUN_00251bb0();
    uVar4 = in_ECX[9] ^ *(uint *)(in_ECX[0x708] + 0x44);
    while ((uVar4 & 0xfffffff) != 0) {
      FUN_00251730();
      uVar4 = in_ECX[9] ^ *(uint *)(in_ECX[0x708] + 0x44);
    }
  }
  do {
    iVar3 = FUN_00259160();
  } while (iVar3 != 0);
  FUN_00255520();
  FUN_00251890();
  puVar1 = (uint *)in_ECX[0x1e8];
  if ((puVar1 != (uint *)0x0) &&
     (uVar4 = *puVar1, *puVar1 = uVar4 - 0x80000, (uVar4 - 0x80000 & 0x78ffff) == 0)) {
    FUN_00257b40(puVar1);
  }
  piVar5 = in_ECX + 0x3e6;
  iVar3 = 4;
  do {
    puVar1 = (uint *)piVar5[-4];
    if ((puVar1 != (uint *)0x0) &&
       (uVar4 = *puVar1, *puVar1 = uVar4 - 0x80000, (uVar4 - 0x80000 & 0x78ffff) == 0)) {
      FUN_00257b40(puVar1);
    }
    puVar1 = (uint *)*piVar5;
    if ((puVar1 != (uint *)0x0) &&
       (uVar4 = *puVar1, *puVar1 = uVar4 - 0x80000, (uVar4 - 0x80000 & 0x78ffff) == 0)) {
      FUN_00257b40(puVar1);
    }
    piVar5 = piVar5 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  uVar4 = 0;
  do {
    puVar1 = *(uint **)((int)&DAT_00263700 + uVar4);
    if ((puVar1 != (uint *)0x0) &&
       (uVar2 = *puVar1, *puVar1 = uVar2 - 0x80000, (uVar2 - 0x80000 & 0x78ffff) == 0)) {
      FUN_00257b40(puVar1);
    }
    uVar4 = uVar4 + 0xc;
  } while (uVar4 < 0xc0);
  piVar5 = in_ECX + 0x1f5;
  iVar3 = 0x40;
  do {
    if (*piVar5 != 0) {
      FUN_00160a47(*piVar5,0xa1800000);
    }
    piVar5 = piVar5 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  if (in_ECX[0x922] != 0) {
    FUN_00160a47(in_ECX[0x922],0xa1800000);
  }
  if (in_ECX[0x1e9] != 0) {
    FUN_00160a47(in_ECX[0x1e9],0x24800000);
  }
  if (in_ECX[0x70a] != 0) {
    *(undefined4 *)(in_ECX[0x70a] + 0x140) = 0;
    FUN_0025a606();
  }
  puVar6 = &DAT_002636f8;
  for (iVar3 = 0x30; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar6 = 0;
    puVar6 = puVar6 + 1;
  }
  return;
}


// callers: 0x255bf0

void FUN_00255450(void)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  uint uVar4;
  
  iVar1 = DAT_00261238;
  uVar2 = 0;
  do {
    FUN_00252af0(uVar2,&DAT_0025eef8);
    uVar2 = uVar2 + 1;
  } while (uVar2 < 10);
  uVar2 = 0x39;
  puVar3 = &DAT_0025f328;
  do {
    if (((uVar2 != 0x9a) && (uVar2 != 0x7e)) && (uVar2 != 0x7f)) {
      FUN_00250080(uVar2,*puVar3);
    }
    uVar2 = uVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (uVar2 < 0xa6);
  FUN_00250080(0x8f,*(int *)(iVar1 + 0x1a20) != 0);
  uVar2 = 0;
  do {
    uVar4 = 0;
    do {
      FUN_00250ac0(uVar2,uVar4,(&DAT_0025f308)[uVar4]);
      uVar4 = uVar4 + 1;
    } while (uVar4 < 0x20);
    FUN_00250ac0(uVar2,0x1c,uVar2);
    FUN_00253800(uVar2,0);
    uVar2 = uVar2 + 1;
  } while (uVar2 < 4);
  FUN_00250ac0(0,0xc,4);
  FUN_00250ac0(0,0x10,2);
  FUN_002541d0(0,0);
  return;
}


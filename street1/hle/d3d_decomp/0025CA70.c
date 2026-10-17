// callers: 0x1a0860

void FUN_0025ca70(undefined4 param_1,uint param_2,uint param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  uint uVar4;
  
  puVar1 = DAT_00261238;
  FUN_00257950(0);
  uVar4 = param_3 - 1 >> 8;
  puVar2 = (undefined4 *)FUN_00252050(uVar4 + 6);
  *puVar2 = 0x417fc;
  puVar2[1] = param_1;
  puVar2[2] = (uVar4 + 1) * 0x40000 + 0x40001810;
  if (0x100 < param_3) {
    iVar3 = (param_3 - 0x101 >> 8) + 1;
    do {
      puVar2[3] = param_2 | 0xff000000;
      puVar2 = puVar2 + 1;
      param_2 = param_2 + 0x100;
      param_3 = param_3 - 0x100;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
  }
  puVar2[3] = (param_3 - 1) * 0x1000000 | param_2;
  puVar2[4] = 0x417fc;
  puVar2[5] = 0;
  *puVar1 = puVar2 + 6;
  return;
}

